// -*- coding: utf-8; mode: c++; tab-width: 3 -*-
//--------------------------------------------------------------------------------------------------
// Application-Building Components
// Copyright 2013 Raffaello D. Di Napoli
//--------------------------------------------------------------------------------------------------
// This file is part of Application-Building Components (henceforth referred to as ABC).
//
// ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ABC. If not, see
// <http://www.gnu.org/licenses/>.
//--------------------------------------------------------------------------------------------------

// Persistent storage support (a.k.a. no-SQL database).


/* TODO: refine design and implement :)

General design:

•	“Server-domain” denotes a component that should be runnable indifferently on the same machine as
	the client, or on a remote machine.

•	Server-domain UID generator; each instance of a persistable class will get an UID upon
	construction.

•	When a pointer to a persistable class is needed, clients must use special persistable shared
	smart pointers which have both UID and memory address (if loaded) of the target object.

•	When a persistable pointer is accessed via such smart pointer and the pointer’s memory address is
	NULL but the UID is non-zero, a “load” class method of the pointed-to class (it’s a smart
	pointer, so it knows) is invoked, returning a real instance which is then bound to the pointer
	which, being shared, automatically makes the object visible to every client holding a reference
	to it.

•	The “load” class method of a persistable class sends a request to the server-domain storage
	server, providing the instance’s UID and the name of each persistable scalar data member (string,
	integer, float, blob). Empty/default data is never stored; defaulting is handled on the client
	side, in the translation layer between persistable class and storage server.

	All non-scalar persistable data members of a persistable class have to use the persistable shared
	smart pointer described above; this allows to have on-demand loading for persistable class
	hierarchies.

•	The “unload” class method of a persistable class - TODO: when is this invoked?

•	When the reference count of a persistable shared smart pointer drops to zero, the “free” class
	method of the persistable class is invoked to discard the storage server’s data related to the
	object.

•	Caching:
	•	Client read caching: once an object is loaded, the class instance acts as a local cache for
		the object.

	•	Client write caching: TODO: perhaps a class can decide whether writes to persistable
		members are written immediately (write-through) or only when the “unload” method is called?

	•	Server read caching: TODO: while the server might not have a use for a read cache, it needs
		a special preload cache, discarded quickly if not used. The server probably also needs a row
		metadata cache, to keep in-memory metadata for the most recently/often used rows - see below.

	•	Server write caching: TODO: maybe writes should be grouped in batches, to reduce the number
		of waits for locks when a large number of rows is written at once?

•	The server-domain storage server stores (UID, key, value) triplets (rows).

	•	Sequential access speed-up: since most keys will be always be read in the same sequence order
		due to being part of the same persistable class instance, each row will hold a reference to a
		“preload” row that should be loaded while the data for the current row is processed (after
		being loaded) and returned. Ideally, this should allow to load an object of any size keeping
		the highest parallelism between disk and processor usage.

	•	Relational access speed-up: since many objects will always be retrieved in the same order due
		to the class hierarchy, the “preload” metadata could be used cross-object.

		However, in order for this to be self-adapting (load order might not be known at design time),
		the storage server should store a weighted list of rows accessed after another one is, storing
		a reference to the heaviest (most likely) row in the “preload” metadata, but also storing and
		constantly updating the weight of other accesses.

		•	This could keep only the top N (e.g. 8) permanently on disk, but as long as the row is in
			the server cache, many more preload candidates can be kept; when the cache needs to be
			flushed, the top N will be updated as well as the top “preload” row metadata.

		•	Weighting: TODO: what makes a good candidate for preloading?
*/

