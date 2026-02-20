# LSFMM 2026 — Filesystem Track: Topics & Attendees

Generated: 2026-02-20
Sources:
- `fsnotify/` mailbox (linux-fsdevel), Jan–Feb 2026
- `LSF_MM_BPF_2026_PLANNING_FS_topics.csv`

Lore links: `https://lore.kernel.org/linux-fsdevel/<msgid>/`
Reply counts from local fsnotify mailbox.

Rating scale:
- **-1** = negative / wrong-venue pushback
- **0** = no response yet
- **1–5** = positive engagement (5 = hottest, most actionable)

Invitation status column key (attendees tables):
- ✅ = requested invitation in CSV
- ⚠️ = needs visa (flagged in CSV)
- ❌ = high relevance but **no invitation requested** — needs to be invited

---

## TOPICS (sorted by interest)

---

### ★★★★★ Score: 5 · 31 replies · CSV: ✅ proposed

**[Where is fuse going? API cleanup, restructuring, and new version?](https://lore.kernel.org/linux-fsdevel/CAJfpegtzYdy3fGGO5E1MU8n+u1j8WVc2eCoOQD_1qq0UV92wRw@mail.gmail.com/)**
Proposed by: **Miklos Szeredi** `mszeredi@redhat.com` — Feb 2, 2026

Umbrella session covering the FUSE roadmap. Confirmed in CSV.

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

**FUSE sub-topics confirmed in CSV (can be absorbed into this session or split off):**

- **FUSE compound requests** — Horst Birthelmer `horst@birthelmer.com` (CSV ✅): group FUSE
  requests into compounds for atomic server-side handling; patch published for review.
  Required: Miklos Szeredi, Luis Henriques, Joanne Koong, Bernd Schubert.

- **FUSE io-uring queue reduction and distribution** — Bernd Schubert `bernd@bsbernd.com`
  (CSV ✅): distribute io-uring queues among multiple FUSE server threads.

- **Race-free umount with an ID** — Bernd Schubert (CSV ✅): FUSE daemon exit edge cases.

- **FUSE servers restartability** — Luis Henriques `luis@igalia.com` (CSV ✅): see
  standalone topic entry below.

- **AI agent filesystem enhancements for overlayfs and erofs** — Gao Xiang `xiang@kernel.org`
  (CSV ✅⚠️ needs visa): tentative topic.

---

### ★★★★☆ Score: 4 · 37 replies · CSV: ✅ proposed (joint session)

**[Buffered atomic writes](https://lore.kernel.org/linux-fsdevel/d0c4d95b-8064-4a7e-996d-7ad40eb4976b@linux.dev/)**
Proposed by: **Pankaj Raghav** `pankaj.raghav@linux.dev` (Samsung) — Feb 13, 2026
Co-lead: **Ojaswin Mujoo** `ojaswin@linux.ibm.com` (IBM) — also confirmed in CSV ✅⚠️

Direct I/O atomic writes landed for ext4/XFS; extension to buffered I/O remains
contested. Concrete PostgreSQL use case from Andres Freund: buffered I/O preferred when
running many Postgres instances sharing the kernel page cache dynamically.

CSV notes:
- Pankaj plans to lead jointly with Andres Freund (MSFT Postgres developer).
- Ojaswin intends to co-present and is unsure whether use-case and design discussions
  should be split into 2 slots.
- Ritesh Harjani `ritesh.list@gmail.com` (CSV ✅⚠️) also requests participation, has
  implemented DIO atomic write iomap changes.

Required per CSV: Andres Freund, XFS Maintainers (Dave Chinner / Darrick J. Wong),
Matthew Wilcox, Christoph Hellwig (from Pankaj); Ritesh Harjani, tytso, djwong,
dchinner (from Ojaswin).

Active mailing list debate on:
- page-locking semantics and DMA races
- crash safety guarantees for the buffered path
- architectural blockers in the current RFCs

---

### ★★★☆☆ Score: 3 · 12 replies · CSV: ❌ not submitted (but related AI topics in CSV)

**[Documenting the correct pushback on AI-inspired patches](https://lore.kernel.org/linux-fsdevel/32e620691c0ecf76f469a21bffaba396f207ccb9.camel@HansenPartnership.com/)**
Proposed by: **James Bottomley** `James.Bottomley@HansenPartnership.com` — Feb 5, 2026
(James Bottomley did **not** submit a CSV invitation request)

Addressing the surge of AI-generated "fix" patches for old drivers and unmaintained
filesystems, where review cost exceeds benefit. Proposal to write a kernel document
("first do no harm") requiring submitters to quantify user-visible impact, likelihood,
and testing evidence.

Supported by: Darrick J. Wong, Matthew Wilcox, Chuck Lever, Bart Van Assche.

**Related CSV topics pulling in the same direction:**
- **Automated AI patch reviews** — Chris Mason `mason.chris.lo@gmail.com` (CSV ✅,
  Btrfs co-maintainer): proposes a session on automating patch review with AI.
- **Agentic LLM integration into review process** — Sasha Levin `sashal@kernel.org`
  (CSV ✅): how AI can better support subsystems (upstream workflow, backports, security).

These three form a natural joint session: Bottomley's doc/policy angle, Mason's
automation angle, and Levin's integration angle. Note that Josef Bacik (CSV ✅) also
listed "AI development" as a topic of interest.

---

### ★★★☆☆ Score: 3 · 7 replies · CSV: ✅ proposed (BoF)

**[A common project for file system performance testing](https://lore.kernel.org/linux-fsdevel/b9f6cd20-8f0f-48d6-9819-e0c915206a3f@wdc.com/)**
Proposed by: **Hans Holmberg** `hans.holmberg@wdc.com` (WDC) — Feb 12, 2026

CSV confirms and expands: Hans is willing to host; requests Jan Kara and Josef Bacik
(fsperf maintainer) as participants. Also proposes a co-host from storage
(e.g. Javier Gonzalez, Keith Busch) for the data placement angle. Uses
`fsperf` (https://github.com/josefbacik/fsperf) as a base.

Required per CSV: Jan Kara, Josef Bacik.

Should be merged or co-scheduled with Theodore Tso's "File system testing" topic below.

---

### ★★★☆☆ Score: 3 · 4+ replies · CSV: ✅ proposed

**[File system testing](https://lore.kernel.org/linux-fsdevel/20260218150736.GD45984@macsyma-wired.lan/)**
Proposed by: **Theodore Tso** `theodore.tso@gmail.com` — Feb 18, 2026

CSV confirms: Ted is willing to lead this session.

Three-point proposal:
1. Standardize test scenario sets per filesystem in the kernel source tree.
2. Encode expected failures per scenario/kernel-version in xfstests upstream.
3. Automate test-based validation of FS backports to LTS kernels.

Required per CSV: **Zorro Lang** (xfstests maintainer), **Jan Kara**, **Darrick Wong**,
**Luis Chamberlain** (mcgrof).

Also: Ritesh Harjani (CSV ✅⚠️) wants to discuss standardizing known xfstests failures
(blocksize < pagesize regressions etc.).

---

### ★★★☆☆ Score: 3 · 5 replies · CSV: ✅ (part of FUSE session)

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

### ★★★☆☆ Score: 3 · 10 replies · CSV: ✅ proposed

**[VFS idmappings support in NFS](https://lore.kernel.org/linux-fsdevel/65a53a2d6fcc053edeed688a8c8d580c03bd6f3b.camel@mihalicyn.com/)**
Proposed by: **Aleksandr Mikhalitsyn** `aleksandr.mikhalitsyn@futurfusion.io` — Feb 18, 2026

Extending `FS_ALLOW_IDMAP` to NFS. Key challenge: network FSes need caller UID/GID in
every RPC, but VFS idmap API restricts `mnt_idmap *` to semantically appropriate ops.
Prior work on FUSE/virtiofs and CephFS (required CephFS protocol change).

Optional per CSV: Kohei Sugihara `ksugihara@preferred.jp`, Toru Komatsu `utam0k@preferred.jp`.

Can be co-scheduled with "Namespace-aware upcalls" as a joint network FS session.

---

### ★★★☆☆ Score: 3 · 7 replies · CSV: ✅ proposed (needs visa ⚠️)

**[Namespace-aware upcalls from kernel filesystems](https://lore.kernel.org/linux-fsdevel/CANT5p=rDxeYKXoCJoWRwGGXv4tPCM2OuX+US_G3hm_tL3UyqtA@mail.gmail.com/)**
Proposed by: **Shyam Prasad N** `sprasad@microsoft.com` — Feb 14, 2026

SMB/NFS upcalls (DNS, auth, ID mapping) are not namespace-aware; always hit init
namespace. Container deployments need upcalls to execute inside the container. Two
approaches: namespace-aware keys or devfs FD-based registration.

Required per CSV: **Christian Brauner**, **David Howells**.
Note: Shyam requests early notification due to visa processing time.

---

### ★★★☆☆ Score: 3 · (new, from CSV)

**[Data placement using userspace hints](https://lore.kernel.org/linux-fsdevel/b9f6cd20-8f0f-48d6-9819-e0c915206a3f@wdc.com/)**
Proposed by: **Hans Holmberg** `hans.holmberg@wdc.com` (WDC) — CSV only

Spans both storage and FS tracks. Discussion goal: align hint semantics across
filesystems and map hints to NVMe FDP placement identifiers.

Complements/extends the FDP write-streams and eBPF data placement topics. Hans proposes
two hosts: one from FS, one from storage (e.g. Javier Gonzalez, Keith Busch).

Required per CSV: Jan Kara, Josef Bacik, Javier Gonzalez, Keith Busch.

---

### ★★☆☆☆ Score: 2 · (new, from CSV)

**[Direct reclaim / filesystem / rwsem scalability](https://lore.kernel.org/linux-fsdevel/)** — CSV only
Proposed by: **Boris Burkov** `boris@bur.io` — CSV ✅

Scalability issues from interactions between direct reclaim, filesystems, and CPU
scheduling — particularly around rwsem. Boris is willing to lead but is primarily
seeking ideas, prior art, and a path forward. Relevant to BTRFS, XFS, and the VFS
locking stack broadly.

---

### ★★☆☆☆ Score: 2 · (new, from CSV)

**[NFS BoF](https://lore.kernel.org/linux-nfs/)** — CSV only
Proposed by: **Anna Schumaker** `anna@kernel.org` (NFS client co-maintainer) — CSV ✅

Standing NFS BoF session. Anna requests one "nice to have." Natural home for
namespace-aware upcalls and VFS idmappings discussions.

---

### ★★☆☆☆ Score: 2 · 10 replies · CSV: ❌ not submitted

**[Refactor LSM hooks for VFS mount operations](https://lore.kernel.org/linux-fsdevel/CAPhsuW4=heDwYEkmRzSnLHDdW=da71qDd1KqUj9sYUOT5uOx3w@mail.gmail.com/)**
Proposed by: **Song Liu** `song@kernel.org` — Jan 21, 2026
(Song Liu did **not** submit a CSV invitation request)

Current LSM hooks lack coverage for VFS mount operations. Paul Moore (LSM maintainer)
pushed back on venue but discussion was constructive. Song confirmed RFC patches will
be posted before the conference.

Conditional: include only if RFC posted and reviewed. Requires LSM representation.

---

### ★★☆☆☆ Score: 2 · 2 replies · CSV: ✅ (Jeff Layton in CSV)

**[Should we make inode->i_ino a u64?](https://lore.kernel.org/linux-fsdevel/08f8444c7237566ffb4ba8c9eb0ab4b4a5f14440.camel@kernel.org/)**
Proposed by: **Jeff Layton** `jlayton@poochiereds.net` — Feb 18, 2026

`inode->i_ino` is `unsigned long` (32-bit on 32-bit arches), forcing NFS, CIFS, XFS,
CephFS, FUSE to hash/fold 64-bit inodes. `kstat.ino` and `statx.stx_ino` already `u64`.
Largely mechanical on 64-bit; main concern is struct inode growth on 32-bit arches.

---

### ★★☆☆☆ Score: 2 · 2+ replies · CSV: ✅ proposed (needs visa ⚠️)

**[Parallel writeback: design directions and sharding approaches](https://lore.kernel.org/all/20260116100818.7576-1-kundan.kumar@samsung.com/)**
Proposed by: **Kundan Kumar** `kundanthebest@gmail.com` (Samsung) — Feb 16, 2026

Three RFC iterations on parallel writeback (inode-affined → AG-affined → folio AG
tagging). Christoph Hellwig suggested stream-ID-based approach after v3.
Seeking alignment on sharding model and abstraction boundaries.

Required per CSV: **Anuj Gupta** (co-author), **Kanchan Joshi**, **Darrick J. Wong**,
**Christoph Hellwig**.

Ritesh Harjani (CSV ✅⚠️) also wants to discuss from an ext4 perspective.

---

### ★★☆☆☆ Score: 2 · 1 reply · CSV: ✅ proposed

**[FUSE servers restartability](https://lore.kernel.org/linux-fsdevel/87cy301sw8.fsf@wotan.olymp/)**
Proposed by: **Luis Henriques** `luis@igalia.com` (Igalia) — Jan 23, 2026

Restart FUSE server without unmount (crash recovery, live upgrades). Builds on
`FUSE_LOOKUP_HANDLE` WIP. Needs: `NOTIFY_RESEND_LOOKUPS` and `NOTIFY_RESEND_INIT` /
`NOTIFY_REINIT`.

Required per CSV: Amir Goldstein, Miklos Szeredi, Joanne Koong, Darrick J. Wong.
Other requested: Bernd Schubert, Horst Birthelmer.

Absorb into "Where is fuse going?" session.

---

### ★★☆☆☆ Score: 2 · (new, from CSV)

**[ext4 inline data conversion and writepage race conditions](https://lore.kernel.org/all/20250526-ext4_inline_page_mkwrite-v2-1-aa96d9bc287d@igalia.com/)**
Proposed by: **Thadeu Lima de Souza Cascardo** `cascardo@igalia.com` (Igalia) — CSV ✅

Race between ext4 inline data conversion and writepage; RFC patch v2 posted. Wants
face-to-face time for quick design turnarounds on the inline data approach.

Required per CSV: **Jan Kara**, **Theodore Ts'o**.

---

### ★★☆☆☆ Score: 2 · (new, from CSV)

**[VFS distributed file locking / dlmpfs](https://lore.kernel.org/linux-fsdevel/)** — CSV only
Proposed by: **Alexander Aring** `aahringo@redhat.com` (Red Hat) — CSV ✅

Two related topics: a file locking filesystem RFC (`dlmpfs`) and improvements to the
VFS distributed file locking API. Relevant to network FSes (NFS, SMB) and cluster
filesystems.

---

### ★★☆☆☆ Score: 2 · (new, from CSV)

**[configfs lifecycle: kernel-space management](https://lore.kernel.org/all/20251202-configfs_netcon-v1-0-b4738ead8ee8@debian.org/)**
Proposed by: **Breno Leitao** `breno.leitao@gmail.com` — CSV ✅

configfs items currently created only from userspace mkdir. Proposal to allow kernel-
driven creation for boot params, hardware auto-detection (netconsole use case). RFC
posted; contentious topic, wants alignment.

Required per CSV: **Andreas Hindborg**.

---

### ★☆☆☆☆ Score: 1 · 1 reply · CSV: ✅ (Naohiro in CSV, no topics listed)

**[FDP file I/O via write-streams](https://lore.kernel.org/linux-fsdevel/83f2e586-75d8-44a3-8427-ea6165f1dff9@samsung.com/)**
Proposed by: **Kanchan Joshi** `joshi.k@samsung.com` (Samsung) — Feb 20, 2026
(Kanchan Joshi did **not** submit a CSV invitation request; listed as Required by Kundan Kumar)

Block-layer FDP/write-streams (since 6.16) not yet accessible from file I/O. New
ioctl-based VFS stream management + XFS support. VFS portion solid; XFS needs review.

Co-schedulable with eBPF data placement and Data placement hints (Holmberg).

---

### ★☆☆☆☆ Score: 1 · 3 replies · CSV: ✅⚠️ proposed (needs visa)

**[Data placement policy for FDP SSD in Ceph and distributed filesystems](https://lore.kernel.org/linux-fsdevel/b4bbba0993d4c1abd6566d8d508bbb47aacd7671.camel@ibm.com/)**
Proposed by: **Viacheslav Dubeyko** `slava@dubeyko.com` (IBM) — Jan 23, 2026

FDP for distributed FSes like Ceph; "temperature" hints don't map to CRUSH algorithm.
Needs a new approach for distributed placement. Limited engagement.

---

### ★☆☆☆☆ Score: 1 · 17 replies (many self-replies) · CSV: ✅⚠️ proposed

**[Machine Learning (ML) library in Linux kernel](https://lore.kernel.org/linux-fsdevel/47d21a6821c4b2d085f7b97bcdaa205bfcb0e0ad.camel@ibm.com/)**
Proposed by: **Viacheslav Dubeyko** `slava@dubeyko.com` — Feb 6, 2026

Jan Kara: *"I'm skeptical about how generic this can be... ML is a userspace business
here... prone to deadlocks, security issues."* Barry Song, Chris Li, SeongJae Park
engaged. Not a primary FS topic.

Note from CSV: Dubeyko acknowledges this may fit better in storage, MM, or BPF tracks.

---

### ★☆☆☆☆ Score: 1 · (new, from CSV)

**[64k / 16k base page size on x86](https://lore.kernel.org/linux-fsdevel/)** — CSV only
Proposed by: **Ritesh Harjani** `ritesh.list@gmail.com` (IBM) — CSV ✅⚠️

Wants to shed light from the PowerPC (64K default pagesize) perspective on x86 larger
base page size discussions. Cross-subsystem (MM + FS).

---

### ☆☆☆☆☆ Score: 0 · 1 reply · CSV: ✅⚠️ proposed

**[Large folio buffered I/O: iomap vs filesystem-local implementations](https://lore.kernel.org/linux-fsdevel/75f43184.d57.19c7b2269dd.Coremail.nzzhao@126.com/)**
Proposed by: **Nanzhe Zhao** `nzzhao@126.com` — Feb 20, 2026

Focus: per-folio subrange state (iomap API vs fs-local) and COW/out-of-place writeback
support. Nanzhe can moderate; wants co-leads.

Optional per CSV: Christoph Hellwig, Matthew Wilcox, Jaegeuk Kim, Yi Zhang.

---

### ☆☆☆☆☆ Score: 0 · 1 reply · CSV: ✅ (no topics listed in CSV)

**[eBPF-driven Data Placement Hint](https://lore.kernel.org/linux-fsdevel/DGJLR6FR7XLX.1D72IEB0DX9KC@wdc.com/)**
Proposed by: **Naohiro Aota** `naohiro.aota@wdc.com` (WDC) — Feb 20, 2026

BPF at `file_open` to set data steering (dir, process, cgroup) for zone/block-group
selection. Use cases: ZBD GC, multi-tenant isolation, HDD zoning. Posted today.

---

### ☆☆☆☆☆ Score: 0 · 2 replies · CSV: ✅⚠️ proposed

**[Support to split superblocks during remount](https://lore.kernel.org/linux-fsdevel/CANT5p=orpQdzqxjNronnnKUo5HFGjuVwkwpjiGHQRmwh8es0Pw@mail.gmail.com/)**
Proposed by: **Shyam Prasad N** `sprasad@microsoft.com` — Feb 17, 2026

SMB multi-share superblock splitting during remount. Niche CIFS/VFS issue.
Required per CSV: **David Howells**.

---

### ✗ Score: -1 · 13 replies · CSV: ✅⚠️ proposed

**[Is it time of refreshing interest to NILFS2?](https://lore.kernel.org/linux-fsdevel/8e6c3a70db8b216ab3e9aba1a485de8e6e9db23d.camel@ibm.com/)**
Proposed by: **Viacheslav Dubeyko** `slava@dubeyko.com` — Jan 23, 2026

Theodore Tso: *"This might be better fit for the Linux Plumbers Conference. LSF/MM/BPF
is focused on discussions, not talks. Fresh Linux kernel developers are unlikely to be
there."*

**Recommendation: decline / redirect to LPC.**

---

---

## ATTENDEES

### Required — Filesystem Track

Ranked by relevance (5 = essential). Invitation column: ✅ in CSV · ❌ not in CSV.

| Rank | Inv | Person | Email | Affiliation | MAINTAINERS Role | Key Topics |
|------|-----|--------|-------|-------------|-----------------|------------|
| **5** | ❌ | **Christian Brauner** | `brauner@kernel.org` | — | M: FILESYSTEMS (VFS), IOMAP, IDMAPPED MOUNTS | All VFS topics; idmappings; **required by Shyam** |
| **5** | ✅ | **Miklos Szeredi** | `mszeredi@redhat.com` | Red Hat | M: FUSE, OVERLAY FS | Proposed FUSE direction + xattr caching |
| **5** | ❌ | **Jan Kara** | `jack@suse.cz` | SUSE | M: FANOTIFY, INOTIFY; R: VFS | Buffered atomics, FUSE, xattr; **required by Holmberg, tytso, Cascardo** |
| **5** | ❌ | **Darrick J. Wong** | `djwong@kernel.org` | — | R: IOMAP; XFS contributor | Buffered atomics, FUSE, FS testing; **required by Luis, Ojaswin, tytso, Kundan** |
| **5** | ❌ | **Christoph Hellwig** | `hch@lst.de` | — | M: IOMAP subsections | Buffered atomics, parallel writeback; **required by Pankaj, Kundan; optional Nanzhe** |
| **5** | ❌ | **Dave Chinner** | `dchinner@redhat.com` | Red Hat | XFS core contributor | Buffered atomics; **required by Ojaswin, implied by Pankaj** |
| **4** | ✅ | **Theodore Tso** | `theodore.tso@gmail.com` | Google/MIT | M: EXT4 | Proposed FS testing; required by Ojaswin, Cascardo |
| **4** | ✅ | **Jeff Layton** | `jlayton@poochiereds.net` | Red Hat | M: NFSD, EXPORTFS | Proposed i_ino u64; idmappings; namespace upcalls |
| **4** | ✅ | **Chuck Lever** | `chuck.lever@oracle.com` | Oracle | M: NFSD, EXPORTFS | VFS idmappings, namespace upcalls, AI pushback |
| **4** | ❌ | **Amir Goldstein** | `amir73il@gmail.com` | — | R: FANOTIFY, INOTIFY, OVERLAY FS, EXPORTFS | FUSE direction, buffered atomics; **required by Luis Henriques** |

---

### Optional — Filesystem Track

| Rank | Inv | Person | Email | Affiliation | MAINTAINERS Role | Key Topics |
|------|-----|--------|-------|-------------|-----------------|------------|
| **4** | ❌ | **Matthew Wilcox** | `willy@infradead.org` | Oracle | M: XArray, page cache | AI pushback, buffered atomics, large folios; **required by Pankaj; optional Nanzhe** |
| **4** | ✅ | **Carlos Maiolino** | `cem@kernel.org` | SUSE | M: XFS | Write-streams, parallel writeback, buffered atomics, iomap |
| **4** | ✅ | **Josef Bacik** | `josef@toxicpanda.com` | Meta | M: BTRFS | fsperf owner; required by Holmberg; AI development interest |
| **4** | ✅ | **Anna Schumaker** | `anna@kernel.org` | — | M: NFS CLIENT | Proposed NFS BoF; NFS idmappings; namespace upcalls |
| **3** | ✅ | **Pankaj Raghav** | `pankaj.raghav@linux.dev` | Samsung | — | Proposed buffered atomic writes |
| **3** | ✅ | **Andres Freund** | `andres@anarazel.de` | Microsoft | — | PostgreSQL use case for buffered atomics |
| **3** | ✅ | **Joanne Koong** | `joannelkoong@gmail.com` | Meta | — | FUSE io_uring zero-copy; FUSE direction |
| **3** | ✅ | **Luis Henriques** | `luis@igalia.com` | Igalia | — | Proposed FUSE restartability; FUSE direction |
| **3** | ✅ | **Gao Xiang** | `xiang@kernel.org` | Alibaba | M: EROFS | FUSE direction; AI/overlayfs/erofs ⚠️ visa |
| **3** | ✅ | **Bernd Schubert** | `bernd@bsbernd.com` | — | — | FUSE io-uring, umount, compound requests |
| **3** | ✅ | **Shyam Prasad N** | `sprasad@microsoft.com` | Microsoft | R: CIFS/SMB3 | Namespace upcalls + superblock split ⚠️ visa |
| **3** | ✅ | **Aleksandr Mikhalitsyn** | `aleksandr.mikhalitsyn@futurfusion.io` | — | — | Proposed VFS idmappings for NFS |
| **3** | ✅ | **Hans Holmberg** | `hans.holmberg@wdc.com` | WDC | — | FS benchmarking + data placement hints |
| **3** | ✅ | **Boris Burkov** | `boris@bur.io` | Meta | — | Direct reclaim / rwsem scalability (BTRFS, XFS) |
| **3** | ❌ | **James Bottomley** | `James.Bottomley@HansenPartnership.com` | — | M: SCSI | Proposed AI pushback doc; not in CSV |
| **3** | ✅ | **Chris Mason** | `mason.chris.lo@gmail.com` | Meta | M: BTRFS | Automated AI patch reviews (related to Bottomley) |
| **3** | ✅ | **Ojaswin Mujoo** | `ojaswin@linux.ibm.com` | IBM | — | Buffered atomics co-lead ⚠️ visa |
| **3** | ✅ | **Ritesh Harjani** | `ritesh.list@gmail.com` | IBM | — | Buffered atomics, FS testing, parallel writeback ⚠️ visa |
| **2** | ✅ | **Kundan Kumar** | `kundanthebest@gmail.com` | Samsung | — | Proposed parallel writeback ⚠️ visa |
| **2** | ❌ | **Kanchan Joshi** | `joshi.k@samsung.com` | Samsung | — | Proposed FDP write-streams; **required by Kundan** |
| **2** | ✅ | **Naohiro Aota** | `naohiro.aota@wdc.com` | WDC | — | Proposed eBPF data placement (no CSV topics listed) |
| **2** | ❌ | **Song Liu** | `song@kernel.org` | Meta | — | Proposed LSM hooks for VFS mounts; not in CSV |
| **2** | ❌ | **Paul Moore** | `paul@paul-moore.com` | — | M: LSM | Required if Song's LSM topic accepted; not in CSV |
| **2** | ✅ | **Nanzhe Zhao** | `nzzhao@126.com` | — | — | Large folio / iomap ⚠️ visa |
| **2** | ✅ | **David Howells** | `dhowells@redhat.com` | Red Hat | M: NETFS LIBRARY | Namespace upcalls; **required by Shyam (both topics)** |
| **2** | ✅ | **Thadeu Cascardo** | `cascardo@igalia.com` | Igalia | — | ext4 inline data/writepage races |
| **2** | ✅ | **Alexander Aring** | `aahringo@redhat.com` | Red Hat | — | File locking / dlmpfs / VFS distributed locking |
| **2** | ✅ | **Horst Birthelmer** | `horst@birthelmer.com` | — | — | FUSE compound requests (patch posted) |
| **2** | ✅ | **Sasha Levin** | `sashal@kernel.org` | — | — | Agentic LLM for kernel review workflow |
| **1** | ❌ | **Chao Yu** | `chao@kernel.org` | Huawei | M: F2FS, EROFS | FDP, large folios; not in CSV |
| **1** | ❌ | **Ilya Dryomov** | `idryomov@gmail.com` | — | M: CEPH | FDP/Ceph data placement; not in CSV |
| **1** | ✅ | **Viacheslav Dubeyko** | `slava@dubeyko.com` | IBM | M: NILFS2, CEPH | Proposed 3 topics (NILFS2 -1, ML ±, FDP/Ceph +) ⚠️ visa |
| **1** | ✅ | **Paulo Alcantara** | `pc@manguebit.org` | — | R: CIFS/SMB3 (DFS), NETFS | CIFS/network FS; no topics listed |
| **1** | ✅ | **Johannes Thumshirn** | `johannes.thumshirn@wdc.com` | WDC | — | XFS/ZBD/btrfs contributor; no topics listed |
| **1** | ✅ | **Aleksa Sarai** | `cyphar@cyphar.com` | — | — | VFS/openat2/pathname contributor; no topics listed |
| **1** | ✅ | **André Almeida** | `andrealmeid@igalia.com` | Igalia | — | Case-insensitive FS, overlayfs topics |
| **1** | ✅ | **Omar Sandoval** | `osandov@osandov.com` | — | — | FUSE (Miklos'), blktests author |
| **1** | ✅ | **Breno Leitao** | `breno.leitao@gmail.com` | — | — | configfs kernel-space lifecycle |

---

## ❌ HIGH RELEVANCE — NO INVITATION REQUESTED

These developers are cited as **required** or **strongly desired** by topic proposers
in the CSV, and/or are ranked highly in the topic analysis, but have **not submitted an
invitation request**. They should be proactively invited by the program committee.

| Priority | Person | Email | Cited as Required By | Topics |
|----------|--------|-------|----------------------|--------|
| 🔴 **Critical** | **Christian Brauner** | `brauner@kernel.org` | Shyam Prasad N (namespace upcalls) | VFS/FILESYSTEMS maintainer; central to almost every FS topic |
| 🔴 **Critical** | **Jan Kara** | `jack@suse.cz` | Holmberg (FS benchmarking), tytso (FS testing), Cascardo (ext4 inline) | VFS reviewer; FANOTIFY/INOTIFY maintainer; broad relevance |
| 🔴 **Critical** | **Darrick J. Wong** | `djwong@kernel.org` | Luis Henriques, Ojaswin Mujoo, tytso, Kundan Kumar | IOMAP; buffered atomics, parallel writeback, FS testing |
| 🔴 **Critical** | **Christoph Hellwig** | `hch@lst.de` | Pankaj Raghav, Kundan Kumar (required); Nanzhe Zhao (optional) | IOMAP; buffered atomics, parallel writeback stream-ID |
| 🔴 **Critical** | **Dave Chinner** | `dchinner@redhat.com` | Ojaswin Mujoo (required); Pankaj Raghav (implied "XFS Maintainers") | XFS; buffered atomics deep design participant |
| 🔴 **Critical** | **Amir Goldstein** | `amir73il@gmail.com` | Luis Henriques (FUSE restartability, required) | FANOTIFY/INOTIFY/overlayfs; FUSE; active in multiple threads |
| 🟠 **Important** | **Matthew Wilcox** | `willy@infradead.org` | Pankaj Raghav (buffered atomics, required); Nanzhe Zhao (optional) | Page cache / large folios / buffered atomics |
| 🟠 **Important** | **James Bottomley** | `James.Bottomley@HansenPartnership.com` | — (self-proposed topic on list, not in CSV) | Proposed AI patch quality doc; 12 replies; SCSI maintainer |
| 🟠 **Important** | **Kanchan Joshi** | `joshi.k@samsung.com` | Kundan Kumar (parallel writeback, required) | Proposed FDP write-streams on list; not in CSV |
| 🟡 **Useful** | **Song Liu** | `song@kernel.org` | — (self-proposed LSM hooks topic on list) | LSM hooks for VFS mounts; BPF/FS boundary |
| 🟡 **Useful** | **Paul Moore** | `paul@paul-moore.com` | — | LSM maintainer; needed if LSM hooks topic proceeds |
| 🟡 **Useful** | **Anuj Gupta** | (Samsung) | Kundan Kumar (parallel writeback, required) | Parallel writeback co-author |
| 🟡 **Useful** | **Zorro Lang** | (xfstests maintainer) | Theodore Tso (FS testing, required) | xfstests maintainer |
| 🟡 **Useful** | **Luis Chamberlain** | `mcgrof@kernel.org` | Theodore Tso (FS testing, required) | kmod/sysfs; FS testing automation |
| 🟡 **Useful** | **Chao Yu** | `chao@kernel.org` | — | F2FS/EROFS maintainer; FDP and large folio impact |
| 🟡 **Useful** | **Ilya Dryomov** | `idryomov@gmail.com` | — | Ceph maintainer; FDP/Ceph data placement |

---

## Attendees Who Need Visa — Early Notification Required

These CSV respondents flagged visa requirements. Invitations must go out promptly:

| Person | Email | Topic(s) |
|--------|-------|----------|
| Shyam Prasad N | `sprasad@microsoft.com` | Namespace-aware upcalls, Superblock split |
| Ojaswin Mujoo | `ojaswin@linux.ibm.com` | Buffered atomic writes (co-lead) |
| Ritesh Harjani | `ritesh.list@gmail.com` | Buffered atomics, FS testing, parallel writeback |
| Kundan Kumar | `kundanthebest@gmail.com` | Parallel writeback |
| Viacheslav Dubeyko | `slava@dubeyko.com` | ML library, FDP/Ceph, NILFS2 |
| Gao Xiang | `xiang@kernel.org` | AI/overlayfs/erofs FUSE enhancements |
| Nanzhe Zhao | `nzzhao@126.com` | Large folio / iomap |

---

## Session Grouping Suggestions

| Session | Topics | Key Required People Not Yet Invited |
|---------|--------|--------------------------------------|
| **FUSE 2026** | "Where is fuse going?" + restartability + xattr caching + compound requests + io-uring queues | Amir Goldstein ❌ |
| **Buffered Atomic Writes** | Standalone (37 replies; may need 2 slots: use-case + design) | Darrick J. Wong ❌, Christoph Hellwig ❌, Dave Chinner ❌, Matthew Wilcox ❌ |
| **FS Testing & Benchmarking** | FS testing (tytso) + FS perf benchmarking (Holmberg) | Jan Kara ❌, Zorro Lang ❌, Luis Chamberlain ❌ |
| **Data Placement** | FDP write-streams + eBPF data placement + userspace hints + FDP/Ceph | Kanchan Joshi ❌, Christian Brauner ❌ |
| **VFS / Network FS** | VFS idmappings in NFS + namespace upcalls + i_ino u64 + NFS BoF + superblock split | Christian Brauner ❌ |
| **AI & Patch Quality** | Bottomley's policy doc + Mason's automation + Levin's LLM review | James Bottomley ❌ |
| **Parallel Writeback** | Standalone or with buffered atomics day 2 | Darrick J. Wong ❌, Christoph Hellwig ❌ |
| **LSM hooks for VFS mounts** | Conditional on RFC; needs LSM track cross-pollination | Song Liu ❌, Paul Moore ❌ |
| **ext4 / VFS edge cases** | ext4 inline data (Cascardo) + direct reclaim/rwsem (Burkov) + 64k page size (Ritesh) | Jan Kara ❌ |
