# LSFMM 2026 — Filesystem Track: Topics & Attendees

Generated: 2026-02-20 · Updated: 2026-03-23
Sources:

- `fsnotify/` mailbox (linux-fsdevel), Jan–Mar 2026
- `LSF_MM_BPF_2026_PLANNING_FS_topics.csv`

Lore links: `https://lore.kernel.org/linux-fsdevel/<msgid>/`
Reply counts from local fsnotify mailbox.

Rating scale:

- **-1** = negative / wrong-venue pushback
- **0** = no response yet
- **1–5** = positive engagement (5 = hottest, most actionable)

Cross-track flags:

- 🔀 **MM** = suggest joint session with MM track
- 🔀 **Storage** = suggest joint session with Storage track
- 🔀 **BPF** = suggest joint session with BPF track
- 🌐 **Remote:** remote participation suggested

---

## TOPICS (sorted by interest)

---

### ★★★★★ Score: 5 · 31 replies

**[Where is fuse going? API cleanup, restructuring, and new version?](https://lore.kernel.org/linux-fsdevel/CAJfpegtzYdy3fGGO5E1MU8n+u1j8WVc2eCoOQD_1qq0UV92wRw@mail.gmail.com/)**
Proposed by: **Miklos Szeredi** `mszeredi@redhat.com` — Feb 2, 2026
🌐 **Remote: Al Viro** — VFS API/mount semantics
🌐 **Remote: Darrick J. Wong** — commented on FUSE API thread

Umbrella session covering the FUSE roadmap.

New features at various stages:

- `fuse4fs`: exporting the iomap interface to userspace
- famfs: export distributed memory
- io_uring zero-copy
- large folios
- file handles on the userspace API
- compound requests (Horst Birthelmer has a patch posted — see FUSE sub-topics below)
- BPF scripts

Cleaner layer separation (transport vs. filesystem; local vs. distributed).

New cleaned-up API version proposal:

- remove async INIT
- no fixed ROOT_ID
- consolidated caching rules
- remove legacy/problematic flags

Unresolved issues:

- locked/writeback folios vs. reclaim/page migration
- strictlimiting vs. large folios

Mailing list participants: Christian Brauner, Darrick J. Wong, Jan Kara, Amir Goldstein,
Joanne Koong, Luis Henriques, Gao Xiang, Bernd Schubert, John Groves.

**FUSE sub-topics (can be absorbed into this session or split off):**

- **FUSE compound requests** — Horst Birthelmer `horst@birthelmer.com`: group FUSE
requests into compounds for atomic server-side handling; patch published for review.
Required: Miklos Szeredi, Luis Henriques, Joanne Koong, Bernd Schubert.
- **FUSE io-uring queue reduction and distribution** — Bernd Schubert `bernd@bsbernd.com`:
distribute io-uring queues among multiple FUSE server threads.
- **Race-free umount with an ID** — Bernd Schubert: FUSE daemon exit edge cases.
- **FUSE servers restartability** — Luis Henriques `luis@igalia.com`: see
standalone topic entry below.

---

### ★★★★☆ Score: 4 · 37 replies

**[Buffered atomic writes](https://lore.kernel.org/linux-fsdevel/d0c4d95b-8064-4a7e-996d-7ad40eb4976b@linux.dev/)**
Proposed by: **Pankaj Raghav** `pankaj.raghav@linux.dev` (Samsung) — Feb 13, 2026
Co-lead: **Ojaswin Mujoo** `ojaswin@linux.ibm.com` (IBM)
🔀 **\[MM\]** — page-locking semantics, crash safety, DMA races are MM territory

Direct I/O atomic writes landed for ext4/XFS; extension to buffered I/O remains
contested. Concrete PostgreSQL use case from Andres Freund: buffered I/O preferred when
running many Postgres instances sharing the kernel page cache dynamically.

Notes:

- Pankaj plans to lead jointly with Andres Freund (MSFT Postgres developer).
- Ojaswin intends to co-present and is unsure whether use-case and design discussions
should be split into 2 slots.
- Ritesh Harjani also requests participation; has implemented DIO atomic write iomap changes.

Key attendees: Christoph Hellwig, Matthew Wilcox.

Active mailing list debate on:

- page-locking semantics and DMA races
- crash safety guarantees for the buffered path
- architectural blockers in the current RFCs

---

### ★★★☆☆ Score: 3 · 12 replies

**[Documenting the correct pushback on AI-inspired patches](https://lore.kernel.org/linux-fsdevel/32e620691c0ecf76f469a21bffaba396f207ccb9.camel@HansenPartnership.com/)**
Proposed by: **James Bottomley** `James.Bottomley@HansenPartnership.com` — Feb 5, 2026

Addressing the surge of AI-generated "fix" patches for old drivers and unmaintained
filesystems, where review cost exceeds benefit. Proposal to write a kernel document
("first do no harm") requiring submitters to quantify user-visible impact, likelihood,
and testing evidence.

Supported by: Darrick J. Wong, Matthew Wilcox, Chuck Lever, Bart Van Assche.

**Related topics pointing in the same direction:**

- **Automated AI patch reviews** — Chris Mason `mason.chris.lo@gmail.com` (Btrfs
co-maintainer): proposes a session on automating patch review with AI.
- **Agentic LLM integration into review process** — Sasha Levin `sashal@kernel.org`:
how AI can better support subsystems (upstream workflow, backports, security).

These three form a natural joint session: Bottomley's doc/policy angle, Mason's
automation angle, and Levin's integration angle. Josef Bacik also listed
"AI development" as a topic of interest.

---

### ★★★☆☆ Score: 3 · 7 replies

**[A common project for file system performance testing](https://lore.kernel.org/linux-fsdevel/b9f6cd20-8f0f-48d6-9819-e0c915206a3f@wdc.com/)**
Proposed by: **Hans Holmberg** `hans.holmberg@wdc.com` (WDC) — Feb 12, 2026
🔀 **\[Storage\]** — benchmarking infra spans FS and Storage (NVMe, ZBD)

Hans is willing to host; requests Jan Kara and Josef Bacik (fsperf maintainer) as
participants. Also proposes a co-host from storage (e.g. Javier Gonzalez, Keith Busch)
for the data placement angle. Uses `fsperf` ([https://github.com/josefbacik/fsperf](https://github.com/josefbacik/fsperf)) as
a base.

Should be merged or co-scheduled with Theodore Tso's "File system testing" topic below.

---

### ★★★☆☆ Score: 3 · 4+ replies

**[File system testing](https://lore.kernel.org/linux-fsdevel/20260218150736.GD45984@macsyma-wired.lan/)**
Proposed by: **Theodore Tso** `theodore.tso@gmail.com` — Feb 18, 2026
🔀 **\[Storage\]** — xfstests validation spans block/fs boundary

Ted is willing to lead this session.

Three-point proposal:

1. Standardize test scenario sets per filesystem in the kernel source tree.
2. Encode expected failures per scenario/kernel-version in xfstests upstream.
3. Automate test-based validation of FS backports to LTS kernels.

Desired participants: Zorro Lang (xfstests maintainer), Jan Kara, Darrick J. Wong,
Luis Chamberlain (mcgrof).

Also: Ritesh Harjani wants to discuss standardizing known xfstests failures
(blocksize < pagesize regressions etc.).

---

### ★★★☆☆ Score: 3 · 5 replies

**[xattr caching](https://lore.kernel.org/linux-fsdevel/CAJfpegu0PrfCemFdimcvDfw6BZ2R5=kaZ=Zrt6U5T37W=mfEAw@mail.gmail.com/)**
Proposed by: **Miklos Szeredi** `miklos@szeredi.hu` — Feb 9, 2026

No common xattr caching infrastructure exists:

- `mb_cache` (ext2/4): dedup-focused, not general
- `simple_xattr` (tmpfs/kernfs): no shrinker
- `nfs4_xattr_cache` (NFS): per-inode table wasteful for typical 1-2 xattrs

Proposal: unified utility with inline small values, large xattr support, key/value/list
deduplication, optional shrinker. Cross-cutting: FUSE, NFS, any FS.

Can be absorbed into the "Where is fuse going?" session.

---

### ★★★☆☆ Score: 3 · 10 replies

**[VFS idmappings support in NFS](https://lore.kernel.org/linux-fsdevel/65a53a2d6fcc053edeed688a8c8d580c03bd6f3b.camel@mihalicyn.com/)**
Proposed by: **Aleksandr Mikhalitsyn** `aleksandr.mikhalitsyn@futurfusion.io` — Feb 18, 2026
🌐 **Remote: Al Viro** — VFS idmap API, mount namespaces

Extending `FS_ALLOW_IDMAP` to NFS. Key challenge: network FSes need caller UID/GID in
every RPC, but VFS idmap API restricts `mnt_idmap `* to semantically appropriate ops.
Prior work on FUSE/virtiofs and CephFS (required CephFS protocol change).

Optional participants: Kohei Sugihara `ksugihara@preferred.jp`, Toru Komatsu `utam0k@preferred.jp`.

Can be co-scheduled with "Namespace-aware upcalls" as a joint network FS session.

---

### ★★★☆☆ Score: 3 · 7 replies · needs confirmed lead

**[Namespace-aware upcalls from kernel filesystems](https://lore.kernel.org/linux-fsdevel/CANT5p=rDxeYKXoCJoWRwGGXv4tPCM2OuX+US_G3hm_tL3UyqtA@mail.gmail.com/)**
Proposed by: **Shyam Prasad N** `sprasad@microsoft.com` — Feb 14, 2026
🌐 **Remote: Al Viro** — namespace/keyrings architecture

SMB/NFS upcalls (DNS, auth, ID mapping) are not namespace-aware; always hit init
namespace. Container deployments need upcalls to execute inside the container. Two
approaches: namespace-aware keys or devfs FD-based registration.

Attending participants with this as a stated interest: Christian Brauner, David Howells,
Jeff Layton, Chuck Lever.

---

### ★★★☆☆ Score: 3 · CSV only (no mailing list thread)

**[Data placement using userspace hints]** — CSV only
Proposed by: **Hans Holmberg** `hans.holmberg@wdc.com` (WDC)
🔀 **\[Storage\]** — FDP/NVMe placement spans FS and Storage
🔀 **\[BPF\]** — eBPF-driven placement hint sub-topic

Spans both storage and FS tracks. Discussion goal: align hint semantics across
filesystems and map hints to NVMe FDP placement identifiers.

**eBPF sub-topic:** Naohiro Aota's [eBPF-driven Data Placement Hint](https://lore.kernel.org/all/DGJLR6FR7XLX.1D72IEB0DX9KC@wdc.com/)
(Feb 20, 2026) — BPF at `file_open` to set data steering (dir, process, cgroup) for
zone/block-group selection. Naohiro Aota will attend and can lead this sub-topic.

Desired participants: Jan Kara, Josef Bacik, Javier Gonzalez, Keith Busch.

---

### ★★☆☆☆ Score: 2

**[Direct reclaim / filesystem / rwsem scalability](https://lore.kernel.org/linux-fsdevel/)** — CSV only
Proposed by: **Boris Burkov** `boris@bur.io`
🔀 **\[MM\]** — direct reclaim path is core MM territory; rwsem is kernel/locking

Scalability issues from interactions between direct reclaim, filesystems, and CPU
scheduling — particularly around rwsem. Boris is willing to lead but is primarily
seeking ideas, prior art, and a path forward. Relevant to BTRFS, XFS, and the VFS
locking stack broadly.

---

### ★★☆☆☆ Score: 2 · 2+ replies

**[Parallel writeback: design directions and sharding approaches](https://lore.kernel.org/all/20260216095852.4611-1-kundan.kumar@samsung.com/)**
Proposed by: **Kundan Kumar** `kundanthebest@gmail.com` (Samsung) — Feb 16, 2026
🔀 **\[Storage\]** — sharding model touches NVMe queue/zone layout

Three RFC iterations on parallel writeback (inode-affined → AG-affined → folio AG
tagging). Christoph Hellwig suggested stream-ID-based approach after v3.
Seeking alignment on sharding model and abstraction boundaries.

Christoph Hellwig will be attending and is available for this discussion.

Desired participants: Anuj Gupta (co-author), Kanchan Joshi, Darrick J. Wong,
Christoph Hellwig.

Ritesh Harjani also wants to discuss from an ext4 perspective.

---

### ★★☆☆☆ Score: 2 · 1 reply

**[FUSE servers restartability](https://lore.kernel.org/linux-fsdevel/87cy301sw8.fsf@wotan.olymp/)**
Proposed by: **Luis Henriques** `luis@igalia.com` (Igalia) — Jan 23, 2026

Restart FUSE server without unmount (crash recovery, live upgrades). Builds on
`FUSE_LOOKUP_HANDLE` WIP. Needs: `NOTIFY_RESEND_LOOKUPS` and `NOTIFY_RESEND_INIT` /
`NOTIFY_REINIT`.

Required: Amir Goldstein, Miklos Szeredi, Joanne Koong.
Also requested: Bernd Schubert, Horst Birthelmer.

Absorb into "Where is fuse going?" session.

---

### ★☆☆☆☆ Score: 1

**[64k / 16k base page size on x86](https://lore.kernel.org/linux-fsdevel/)** — CSV only
Proposed by: **Ritesh Harjani** `ritesh.list@gmail.com` (IBM)
🔀 **\[MM\]** — page-size architecture is a core MM decision with FS impact

Wants to shed light from the PowerPC (64K default pagesize) perspective on x86 larger
base page size discussions. Cross-subsystem (MM + FS).

---

### ★☆☆☆☆ Score: 1 · 1 reply

**[FDP file I/O via write-streams](https://lore.kernel.org/linux-fsdevel/83f2e586-75d8-44a3-8427-ea6165f1dff9@samsung.com/)**
Proposed by: **Kanchan Joshi** `joshi.k@samsung.com` (Samsung) — Feb 20, 2026
🔀 **\[Storage\]** — FDP/write-streams is block-layer work with FS interface impact

Block-layer FDP/write-streams (since 6.16) not yet accessible from file I/O. New
ioctl-based VFS stream management + XFS support. VFS portion solid; XFS needs review.

Co-schedulable with eBPF data placement and Data placement hints (Holmberg).

---

### ✗ Removed topics


| Topic                                 | Reason                                                           |
| ------------------------------------- | ---------------------------------------------------------------- |
| NILFS2 revival                        | Score −1; redirected to LPC                                      |
| ML library in Linux kernel            | No FS track relevance                                            |
| Data placement for FDP in Ceph        | Limited engagement; Ceph-specific                                |
| configfs kernel-space management      | No confirmed champion                                            |
| VFS distributed file locking / dlmpfs | No confirmed champion                                            |
| ext4 inline data / writepage races    | No confirmed champion                                            |
| Large folio iomap framework           | No confirmed champion; can be raised in buffered atomics session |
| inode->i_ino u64                      | Already merged upstream                                          |
| NFS BoF                               | No confirmed in-person lead                                      |
| LSM hooks for VFS mount operations    | No LSM track cross-participation confirmed                       |
| Superblock split during remount       | No confirmed champion                                            |


---

---

## ATTENDEES

### Confirmed — Filesystem Track

| Person | Email | Affiliation | Role | Key Topics |
|--------|-------|-------------|------|------------|
| **Miklos Szeredi** | `mszeredi@redhat.com` | Red Hat | M: FUSE, OVERLAY FS | Proposed FUSE direction + xattr caching |
| **Amir Goldstein** | `amir73il@gmail.com` | — | R: FANOTIFY, INOTIFY, OVERLAY FS, EXPORTFS | FUSE restartability; overlayfs; buffered atomics |
| **Jan Kara** | `jack@suse.cz` | SUSE | M: FANOTIFY, INOTIFY; R: VFS | Buffered atomics, FUSE, xattr; FS testing/benchmarking |
| **Christian Brauner** | `brauner@kernel.org` | — | M: FILESYSTEMS (VFS), IOMAP, IDMAPPED MOUNTS | All VFS topics; idmappings; namespace upcalls |
| **Matthew Wilcox** | `willy@infradead.org` | Oracle | M: XArray, page cache | Buffered atomics, direct reclaim/rwsem, 64k page size, AI pushback |
| **Theodore Tso** | `theodore.tso@gmail.com` | Google/MIT | M: EXT4 | Proposed FS testing |
| **Jeff Layton** | `jlayton@poochiereds.net` | Red Hat | M: NFSD, EXPORTFS | VFS idmappings; namespace upcalls |
| **Chuck Lever** | `chuck.lever@oracle.com` | Oracle | M: NFSD, EXPORTFS | VFS idmappings, namespace upcalls, AI pushback |
| **Christoph Hellwig** | `hch@lst.de` | — | M: IOMAP subsections | Buffered atomics, parallel writeback stream-ID |
| **Carlos Maiolino** | `cem@kernel.org` | SUSE | M: XFS | Write-streams, parallel writeback, buffered atomics, iomap |
| **Josef Bacik** | `josef@toxicpanda.com` | Meta | M: BTRFS | fsperf owner; AI development |
| **Pankaj Raghav** | `pankaj.raghav@linux.dev` | Samsung | — | Proposed buffered atomic writes |
| **Andres Freund** | `andres@anarazel.de` | Microsoft | — | PostgreSQL use case for buffered atomics |
| **Joanne Koong** | `joannelkoong@gmail.com` | Meta | — | FUSE io_uring zero-copy; FUSE direction |
| **Luis Henriques** | `luis@igalia.com` | Igalia | — | Proposed FUSE restartability; FUSE direction |
| **Gao Xiang** | `xiang@kernel.org` | Alibaba | M: EROFS | FUSE direction; AI/overlayfs/erofs |
| **Bernd Schubert** | `bernd@bsbernd.com` | — | — | FUSE io-uring, umount, compound requests |
| **Aleksandr Mikhalitsyn** | `aleksandr.mikhalitsyn@futurfusion.io` | — | — | Proposed VFS idmappings for NFS |
| **Hans Holmberg** | `hans.holmberg@wdc.com` | WDC | — | FS benchmarking + data placement hints |
| **Boris Burkov** | `boris@bur.io` | Meta | — | Direct reclaim / rwsem scalability |
| **Chris Mason** | `mason.chris.lo@gmail.com` | Meta | M: BTRFS | Automated AI patch reviews |
| **Ojaswin Mujoo** | `ojaswin@linux.ibm.com` | IBM | — | Buffered atomics co-lead |
| **Ritesh Harjani** | `ritesh.list@gmail.com` | IBM | — | Buffered atomics, FS testing, parallel writeback |
| **David Howells** | `dhowells@redhat.com` | Red Hat | M: NETFS LIBRARY | Namespace upcalls; VFS |
| **Naohiro Aota** | `naohiro.aota@wdc.com` | WDC | — | eBPF data placement hint |
| **Kundan Kumar** | `kundanthebest@gmail.com` | Samsung | — | Proposed parallel writeback |
| **Sasha Levin** | `sashal@kernel.org` | — | — | Agentic LLM for kernel review workflow |
| **Paulo Alcantara** | `pc@manguebit.org` | — | R: CIFS/SMB3 (DFS), NETFS | CIFS/network FS; namespace upcalls; VFS idmappings |
| **Horst Birthelmer** | `horst@birthelmer.com` | — | — | FUSE compound requests (patch posted) |
| **Omar Sandoval** | `osandov@osandov.com` | — | — | FUSE; fs testing |
| **Lennart Poettering** | `lennart@poettering.net` | — | — | General VFS, btrfs, xfs, FUSE |
| **Johannes Thumshirn** | `johannes.thumshirn@wdc.com` | WDC | — | Data placement, parallel writeback, fs benchmarking |
| **Jake Edge** | `jake@lwn.net` | LWN.net | — | Coverage |
| **John Groves** | `john@groves.net` | — | — | famfs |
| **Daan De Meyer** | `daan.j.demeyer@gmail.com` | — | — | VFS, FUSE, btrfs |
| **André Almeida** | `andrealmeid@igalia.com` | Igalia | — | VFS idmappings, overlayfs |


---

## Session Grouping Suggestions

| Session | Cross-track | Lead(s) | Topics | Notes |
|---------|-------------|---------|--------|-------|
| **FUSE BoF** | — | Miklos Szeredi, Luis Henriques, Bernd Schubert, Horst Birthelmer, Gao Xiang, John Groves (famfs) | "Where is fuse going?" + restartability + xattr caching + compound requests + io-uring queues + famfs | 🌐 Al Viro (VFS API) · 🌐 Darrick J. Wong |
| **Buffered Atomic Writes** | 🔀 MM | Pankaj Raghav, Ojaswin Mujoo | Standalone (37 replies; may need 2 slots: use-case + design) | |
| **Parallel Writeback** | 🔀 Storage | Kundan Kumar | Standalone | |
| **FS Testing & Benchmarking** | 🔀 Storage | Theodore Tso, Hans Holmberg | FS testing + FS perf benchmarking | |
| **Data Placement** | 🔀 Storage+BPF | Hans Holmberg; Naohiro Aota (eBPF sub-topic) | FDP write-streams + eBPF hints + userspace hints | |
| **VFS / Network FS** | — | Aleksandr Mikhalitsyn (idmappings); TBD (upcalls) | VFS idmappings in NFS + namespace upcalls | 🌐 Al Viro (VFS) |
| **AI & Patch Quality** | — | Chris Mason, Sasha Levin | Bottomley's policy doc + Mason's automation + Levin's LLM review | |
| **Direct reclaim & page size** | 🔀 MM | Boris Burkov, Ritesh Harjani | Direct reclaim/rwsem scalability + 64k/16k base page size | |
