# Plan: update the HSM wiki page and the HTTPDirFS fanotify POC

Written 2026-11-22. Scratch plan, not for commit.

Inputs:

- Wiki page: `~/src/fsnotify-utils/wiki/Hierarchical-Storage-Management-API.mediawiki`
  (repo `amir73il/fsnotify-utils.wiki`, last edited 2025-04-08)
- POC: `~/src/httpdirfs`, branches from 2022 (`fan_lookup_perm`, `fan_vfs_filter`,
  `fan_hsm_poc`, `fan_pre_modify`, ...)
- Kernel: `~/src/linux` branch `fan_pre_dir_access` (on v7.4-rc3) and its
  status page `~/src/fsnotify-utils/doc/pre-dir-access-status.md`
- Tests: LTP `~/src/ltp` branch `fan_pre_dir_access` (fanotify14, fanotify28)

Scope: lazy populate only (file content and directory entries). Change
tracking, pre-modify events, `FAN_MARK_SYNC` and persistent marks are not being
worked on. The wiki keeps them as future work, and the POC drops them.

## 1. Where things stand (facts the wiki and POC must reflect)

| Topic | 2022 POC / wiki | Now |
|---|---|---|
| Group class | `FAN_CLASS_VFS_FILTER` (POC only) | `FAN_CLASS_PRE_CONTENT` (upstream v6.14); `FAN_CLASS_PRE_CONTENT_FID` = `FAN_CLASS_PRE_CONTENT \| FAN_REPORT_DFID_NAME_TARGET` (WIP, needed for dir events) |
| File content event | `FAN_ACCESS_PERM` + `FAN_PRE_VFS` + range | `FAN_PRE_ACCESS` (upstream v6.14), regular files only, before read / write / mmap / fallocate / truncate |
| Range info | `struct { hdr; __u32 count; __u64 offset; }` | `FAN_EVENT_INFO_TYPE_RANGE`, `struct { hdr; __u32 pad; __u64 offset; __u64 count; }` (since v6.14) |
| Dir lookup event | `FAN_LOOKUP_PERM` / `FAN_PATH_ACCESS` | `FAN_PRE_DIR_ACCESS` (0x00200000, WIP) with dir fid + name, on a dcache miss in path walk, the last component of open(), and before create / remove / rename |
| Dir readdir event | open / access perm on dir | `FAN_PRE_DIR_ACCESS` with dir fid and no name, "populate the whole dir"; an allowed event sets `DCACHE_HSM_ONCE`, so no more dir events until the dentry is evicted |
| Deny with errno | `FAN_ERRNO` response struct (POC) | `FAN_DENY_ERRNO(err)` (upstream v6.14), errno limited to EPERM, EIO, EBUSY, ETXTBSY, EAGAIN, ENOSPC, EDQUOT |
| Fs opt-in | none | `SB_I_ALLOW_HSM`: ext4, xfs, btrfs only |
| Pre-modify | `FAN_PRE_MODIFY`, `FAN_PATH_MODIFY`, `FAN_MARK_SYNC` | not worked on. `FAN_PRE_ACCESS` fires before writes too, so fill-before-modify works, but there is no change tracking |
| Persistent marks | `FAN_MARK_XATTR` / xattr ignore mask | not worked on |
| Evictable marks | v5.19 | upstream (v5.19); `FAN_MARK_IGNORE` new ignore semantics upstream (v6.0) |
| Internal VFS users (ovl, nfsd) | not covered | WIP: never block. In an unpopulated dir, negative lookups fail with ENOENT (`lookup_perm_negative()`); nfsd sends a readdir-style event from `fh_verify()` |

Restrictions of `FAN_CLASS_PRE_CONTENT_FID` that the POC must respect:

- only pre-content events (`FAN_PRE_ACCESS`, `FAN_PRE_DIR_ACCESS`) in the mask.
  No `FAN_OPEN_PERM`, so the old "Uninitialized (deny open)" state cannot be
  done in this group.
- no `FAN_ONDIR` or `FAN_EVENT_ON_CHILD`
- events carry fid info **and** `event->fd` (until the response_id / fd-less
  question is settled; see kernel open question 2)
- `FAN_PRE_DIR_ACCESS` on an inode mark requires a directory

## 2. Wiki update plan

Keep the structure and the motivation sections (Introduction, HSM for Linux,
Resource Utilization, Synchronizing). Rewrite the API and reference
implementation parts.

1. **HSM for Linux / FUSE passthrough.** Still accurate. Optionally mention
   backing files for metadata (`fs/backing-file.c`) and that ovl / FUSE can be
   internal users of an HSM-watched fs.
2. **Existing APIs.** Split into "since v5.1" (`FAN_OPEN_PERM`, leases,
   punch hole) and "since v6.14": `FAN_CLASS_PRE_CONTENT`, `FAN_PRE_ACCESS`
   with range info, `FAN_DENY_ERRNO()`, `SB_I_ALLOW_HSM`. Go through the
   drawbacks list:
   - (4) partial content: solved by range info
   - (5) freeze deadlock: solved, the pre-content hooks run before
     `sb_start_write()` / `mnt_want_write()`
   - (1) and (3) startup scan and directories: addressed by the WIP dir events
   - (2) lost modifications: unchanged, out of scope
3. **New fanotify events and information.** Rename and mark status:
   - `FAN_PRE_ACCESS` and range info: merged in v6.14
   - `FAN_PATH_ACCESS` → `FAN_PRE_DIR_ACCESS`, WIP (branch
     `amir73il/linux fan_pre_dir_access`). Document both flavours (readdir
     without name, lookup with name), handle-once via `DCACHE_HSM_ONCE`, the
     `FAN_CLASS_PRE_CONTENT_FID` group, and the create / remove / rename / open
     coverage.
   - `FAN_PRE_MODIFY`, `FAN_PRE_DIR_MODIFY`, `FAN_MARK_SYNC`: move to a "Future
     work: change tracking" subsection, explicitly not being worked on.
4. **Resource Utilization.** Evictable marks: still accurate. Persistent inode
   marks: mark as a not-implemented idea.
5. **Synchronizing access / Populating object content.** Update with what we
   learned:
   - file content is written through `event->fd` (`FMODE_NONOTIFY`), so it
     does not recurse
   - a NONOTIFY *dirfd* does **not** stop lookup events for `*at()` calls
     relative to it, so the HSM must create directory entries through a mount
     that is not watched, or that has an ignore mount mark (as in fanotify28),
     or reopen the dir by its fid there (`open_by_handle_at()`)
   - an HSM may populate from inside the event: the kernel sends the event
     before taking the parent lock or freeze protection, and repeats the
     lookup afterwards
   - a denied event is not handled, so it is generated again on retry
   - internal users (ovl, nfsd) never block; in an unpopulated dir they get
     ENOENT for negative lookups
6. **Evicting file content.** Still valid with `FAN_PRE_ACCESS` for files.
   Note that it needs `FAN_OPEN_PERM` in a group of a lower class (e.g.
   `FAN_CLASS_CONTENT`) when the HSM group is `FAN_CLASS_PRE_CONTENT_FID`.
   Re-arming directories after eviction is an open kernel issue
   (`DCACHE_HSM_ONCE` lives until dentry eviction).
7. **Reference implementation.**
   - Point to the new branches (section 3): `amir73il/httpdirfs
     fan_pre_dir_access` and kernel `amir73il/linux fan_pre_dir_access`.
     Keep a one-line note that `fan_hsm_poc` (2022) is historical.
   - Cache states: replace "Uninitialized (deny open)" with "entries are
     created by the HSM during the dir event, so a user can only see an entry
     after it exists, and file reads always block in `FAN_PRE_ACCESS` until
     the range is cached".
   - "Tracking local modifications" and "Modified files query": move under
     "Future work" with the TODO markers kept, and say that the current POC
     doesn't implement them.
8. **New section "Status and history".** Table of the RFC postings and
   decisions (copy from the kernel status page, "Posting history"), the LTP
   tests (fanotify14 invalid flags, fanotify28 events), and the open questions
   (response_id / fd-less events, whether a separate class is needed,
   path-less events for ovl / nfsd).

Mechanics: edit the `.mediawiki` file in `~/src/fsnotify-utils/wiki`, preview it
on github, and push to `fsnotify-utils.wiki`. One commit for the API facts
(items 2-6) and one for the reference implementation (items 7-8), so the facts
can go in before the POC branch exists.

## 3. POC update plan (lazy populate only)

### Branches

Keep the 2022 branches untouched as history. A single branch
`fan_pre_dir_access` in `~/src/httpdirfs`, no stack; its `branches` file:

```
BRANCHES=(stable fan_pre_dir_access)
```

`stable` = `master` = release 1.2.4 (`d91bb2b`, 2023-01-11, still Makefile and
UBSan). On top of it:

- the old crash fix ("Fix crashes on access to invalid paths"), still needed in
  fanotify mode for paths that exist on disk but not on the server
- step 1: fanotify mode with `FAN_CLASS_PRE_CONTENT` + `FAN_PRE_ACCESS` for file
  content and a `FAN_CLASS_CONTENT` group with `FAN_ACCESS_PERM | FAN_ONDIR` for
  readdir (works on mainline v6.14+)
- step 2: a single `FAN_CLASS_PRE_CONTENT_FID` group with `FAN_PRE_DIR_ACCESS`
  for readdir and lookup (WIP kernel), falling back to step 1 otherwise

Upstream `origin/master` (1.3.x, meson, rewritten `cache.c` / `link.c`) is a
later, separate rebase.

What to carry from the old stack:

| Old commit | Keep? |
|---|---|
| `f3fb30c` / `0dcd680` build and invalid-path fixes | yes |
| `6584a2e` fanotify mode | yes, the base of everything |
| `2d35cec` xxxat() cache access, `DATA_DIR_fd` | yes, but `DATA_DIR_fd` must come from an unwatched mount (see below) |
| `2fa15f0` sequential read of a large file | yes |
| `a29c357` `handle_access_event()` helper | yes |
| `5137efb` pre-vfs filter events | no, replaced by `FAN_PRE_ACCESS` |
| `833de93` return errno when denying | rewrite for `FAN_DENY_ERRNO()` |
| `bbadb7d` seek holes in metadata map | covered by step 1 (range from the event + `Cache_complete()` on the segment map) |
| `5fbee21` bind mount on cache data dir | yes |
| `04bb66b` mount mark + evictable ignore marks | yes, adapted |
| `1a138fb` persistent ignore marks | no |
| `dbc7cbb` access range event | yes, new range struct |
| `7d67607` deny modifications to partially cached files | no; fill before write instead |
| `b380107` crash-safe cache write | yes |
| `1c4a686` reset invalid cache on first access | yes |
| `54a2724` commands from stdin | not needed at this stage (only used by evict) |
| `e18a574` evict command | not needed at this stage. Later it needs no `FAN_OPEN_PERM` group: take a write lease, remove the `FAN_PRE_ACCESS` ignore mark, re-take the lease, punch the data and reset the metadata |
| `38dc70b` change tracking group | no |

Branch creation, rebases and pushes are git operations for you to run or approve.

### Code changes in `src/fanotify.c`

1. **Defines.** Drop `FAN_CLASS_VFS_FILTER`, `FAN_XATTR_IGNORE_MASK`,
   `FAN_MARK_XATTR`, `FAN_MARK_SYNC`, `FAN_LOOKUP_PERM`, `FAN_PRE_MODIFY`,
   `FAN_PRE_ATTRIB`, `FAN_PRE_VFS` and the local `FAN_ERRNO` response struct.
   Add fallbacks for `FAN_CLASS_PRE_CONTENT` (0x08), `FAN_PRE_ACCESS`
   (0x00100000), `FAN_PRE_DIR_ACCESS` (0x00200000),
   `FAN_EVENT_INFO_TYPE_RANGE` (6) with the range struct, and
   `FAN_DENY_ERRNO()`, guarded by `#ifndef`, like LTP `include/lapi/fanotify.h`.
2. **Group init.** `fanotify_init(FAN_CLASS_PRE_CONTENT, O_RDWR | O_LARGEFILE)`
   in step 1, `FAN_CLASS_PRE_CONTENT_FID` in step 2.
   Ignore marks: `FAN_MARK_IGNORE_SURV | FAN_MARK_EVICTABLE`.
3. **Marks.**
   - Mount mark on the user-facing bind mount:
     `FAN_PRE_ACCESS` (+ `FAN_PRE_DIR_ACCESS`), no `FAN_ONDIR`.
   - Fully cached file: evictable inode ignore mark with `FAN_PRE_ACCESS`.
   - Populated dir: rely on `DCACHE_HSM_ONCE`. An ignore mark with
     `FAN_PRE_DIR_ACCESS` on a dir is untested in the kernel and may need
     `FAN_ONDIR`, which the class forbids. Check first (open question K1).
   - Drop `fanotify_add_data_watch()` / the `FAN_OPEN_PERM` ignore dance and the
     "Uninitialized" state: files are created by the HSM inside the dir event,
     and reads of an uncached range always block in `FAN_PRE_ACCESS`.
4. **HSM's own access to the cache dir (deadlock fix).** The old POC dups the
   first event fd as `DATA_DIR_fd` and does all `*at()` calls relative to it.
   On the current kernel, a NONOTIFY dirfd does not suppress lookup events, so
   `mknodat()` / `mkdirat()` / `openat()` in an unpopulated dir would send
   `FAN_PRE_DIR_ACCESS` to the HSM itself and deadlock. Open `DATA_DIR_fd` on
   the underlying data dir **before** bind-mounting it over itself (the watched
   mount is only the bind mount), or add a private bind mount with an ignore
   mount mark (`FAN_MARK_MOUNT | FAN_MARK_IGNORE_SURV`), as fanotify28 does.
   Remove the "store DATA_DIR_fd from first event" hack in `Data_relpath()`.
5. **Event parsing.** Walk all info records instead of assuming the first one
   is the range:
   - `FAN_EVENT_INFO_TYPE_RANGE`: offset / count (both `__u64`)
   - `FAN_EVENT_INFO_TYPE_DFID_NAME`: the dir handle and the name (lookup
     event); an empty name means readdir
   - `FAN_EVENT_INFO_TYPE_FID`: the child fid, when present

   Relative path: keep `readlink(/proc/self/fd/<event->fd>)` →
   `Data_relpath()`. For dir events, `event->fd` is the directory.
6. **Dir events** (`fan_pre_dir_access` only), in `handle_access_event()`:
   - no name (readdir): `fs_readdir()` as today, creating all cache entries
     for the dir's `LinkTable`
   - with a name (lookup): look the name up in the parent's `LinkTable` and
     create only that entry (`fs_init_cache()` for one child), which resolves
     the old `TODO: populate a single child entry`. If the remote has no such
     link, create nothing and allow; the kernel then returns ENOENT.
   - create / rename / remove events also arrive as lookup events. Lazy
     populate allows them after populating the name, since local
     modifications are not tracked.
7. **File events.** `fs_read(relpath, event->fd, offset, count ?: 1)` as
   today. A pre-write event is handled like a read (fill first, then allow).
   Drop the `-EROFS` "deny modify of partially cached" path.
8. **Responses.** `struct fanotify_response { fd, response }`, with `FAN_ALLOW`
   or `FAN_DENY_ERRNO(e)`. Map download failures to EIO and "not supported" to
   EPERM. Never answer ENOENT or EROFS, which the kernel rejects with EINVAL.
9. **Startup / mount.** Keep the bind-mount + move-mount flow, but set the
   mount mark before the move, as today. Check `SB_I_ALLOW_HSM` support by
   setting the mark and turning EINVAL / EOPNOTSUPP into a clear error
   ("cache dir must be on ext4, xfs or btrfs").
10. **Commands.** Keep `q`. Leave `evict` out of the first version; re-add it
    later with a `FAN_CLASS_CONTENT` group for `FAN_OPEN_PERM`, so the
    lease-based synchronization from the wiki still works.

### Test plan (kvm-xfstests VM)

1. **Build:** `make` in `~/src/httpdirfs`, as the old `run` script expects. It
   copies libssh / libcurl / libgumbo / libubsan and CA certs from `/vtmp`.
2. **Kernels:**
   - mainline v6.14+ for the step 1 fallback;
   - `fan_pre_dir_access` for both branches.
3. **Scenarios** (cache on an xfs or ext4 scratch device; `run` uses
   `/dev/vdf` → `/vdf/cache`, `https://cdn.kernel.org/pub/`):
   - cold `ls` of the mount root and of a subdir: readdir populate, one event
     per dir, then none (`DCACHE_HSM_ONCE`)
   - `stat` / `cat` of a deep path without any readdir: one lookup event per
     component, populating single entries
   - large-file sequential read (the `tar tvfz linux-firmware` from `run`):
     range events, partial fill, then the ignore mark once fully cached
   - restart the HSM with a warm cache: no download, the metadata registry
     is reused
   - write to a cached file: filled first, then allowed
   - `touch` / `mkdir` / `rm` in an unpopulated dir: lookup event, populate,
     allow
   - check `dmesg` for lockdep / hung task, and make sure the HSM never blocks
     on its own access (step 4)
4. **Network:** the guest needs DNS and HTTPS to cdn.kernel.org (`run` sets
   `nameserver 8.8.8.8`). If the VM has no network, serve a local
   `python3 -m http.server` directory listing from the host or the guest.

### Open questions found while planning

- **K1. Ignore marks on directories.** Can `FAN_PRE_DIR_ACCESS` be ignored on a
  dir inode mark without `FAN_ONDIR`, which this class forbids? This is not
  covered by fanotify28. It is needed to keep a populated dir quiet after its
  dentry is evicted, or else the HSM gets a readdir event again and answers
  from its metadata.
- **K2. HSM's own lookups.** Should an `*at()` call relative to an
  `FMODE_NONOTIFY` dirfd (such as `event->fd`) skip the lookup hook? That would
  give HSMs an easy, deadlock-free way to populate. It belongs to the kernel
  question "who modifies directories" (Jan, 2025-07-04).
- **K3. response_id / fd-less events.** If the uapi moves to fd-less events,
  step 5 changes: there's no `event->fd` and no `readlink()`. Paths would
  come from `open_by_handle_at(DATA_DIR_fd, dir fid)`, and file content would
  be written through a NONOTIFY fd opened by the HSM. Keep the event parsing
  in one helper so this stays a local change.
- **P1. Base.** Do the port on the 2022 `master` first, then rebase onto
  upstream `origin/master`, or the other way round? The plan assumes the 2022
  base first.

## 4. Order of work

1. Wiki commit 1 (API facts, items 2-6). Needs no POC.
2. Step 1 (`FAN_PRE_ACCESS`) on `fan_pre_dir_access`, tested on the WIP kernel.
3. Kernel check of K1 (and a decision on K2), for example with a small LTP
   case.
4. `fan_pre_dir_access` branch, tested on the WIP kernel.
5. Wiki commit 2 (reference implementation + status), linking the new branches.
6. Later: rebase onto upstream httpdirfs, `evict` with a `FAN_OPEN_PERM`
   group, and the fd-less port if K3 lands.

## 5. Gaps for HSMs beyond lazy populate

Neither the wiki nor this plan covers these. They matter for an HSM that
exports the backing filesystem directly over SMB or NFS (instead of through a
FUSE passthrough layer) and must hydrate objects, or approve changes, before
any modification. Record them in the wiki's "Future work" section.

### Pre-modify events

- `FAN_PRE_MODIFY` must also fire before metadata changes (chmod, chown,
  utimes, setxattr, removexattr), not only before content changes. The HSM must
  populate the object before a local change, and it doesn't need to know which
  change it is. The old POC had a separate `FAN_PRE_ATTRIB` for this.
- `FAN_PRE_DIR_MODIFY` needs the operation type (create, mkdir, link, unlink,
  rmdir, rename). It must also report a rename as one event with the source
  and target dir fid + name, like the `FAN_RENAME` info records, because
  fanotify has no cookies. `FAN_PRE_DIR_ACCESS` with a name fires before
  create / remove / rename but doesn't say which.
- An HSM that throttles writes needs range info on `FAN_PRE_MODIFY`.
- Bits: the old `fan_pre_modify` HTTPDirFS POC used `0x00200000` for
  `FAN_PRE_MODIFY`, which is now `FAN_PRE_DIR_ACCESS`. `0x00400000` and
  `0x00800000` are free.

Prior art: the kernel branch `fsnotify_pre_modify` (on v6.12) has POC
"intent" hooks for in-kernel listeners, intended for change tracking:

- blocking events from the `mnt_want_write_path()`, `mnt_want_write_name()`,
  `mnt_want_write_rename()`, `mnt_want_write_file()` and `file_start_write()`
  wrappers, sent without filesystem locks held, so a listener can modify the
  filesystem first
- object events before truncate, `O_TRUNC`, write, fallocate, chmod, chown,
  utimes and xattr changes; name events before create, mkdir, mknod, symlink,
  link, unlink and rmdir; rename sends source and target name events that share
  a cookie
- nfsd setattr, write, create, link, rename and unlink call the same hooks

Limits to address before exposing them through fanotify:

- no operation type, new values or range
- `file_start_write()` is void, so `write()`, `ftruncate()` and the
  `copy_file_range()` target can be delayed but not denied
- the internal bits overload `0x00080000` (the reserved `FS_DIR_MODIFY`),
  `0x01000000` and `0x02000000` (`FAN_MNT_ATTACH`, `FAN_MNT_DETACH`)
- they cover common cases, not every modification path

### nfsd must block on pre-modify events

The WIP rule that internal users (ovl, nfsd) never block fits lazy populate.
For an HSM that exports the backing filesystem over NFS, nfsd must wait for
pre-modify events (and ideally for pre-content events) like any other caller,
or NFS clients bypass the HSM. The `fsnotify_pre_modify` hooks do block nfsd.

### More event info and hooks

- **Open flags in `FAN_OPEN_PERM`:** the access mode, `O_TRUNC` and
  `O_CREAT`, so the HSM can act on open for write only (for example to take a
  remote lock) without a separate group.
- **Caller credentials:** uid and gid of the caller as an info record. For
  nfsd the event pid is a kernel thread, so only the credentials identify the
  client.
- **`lseek(SEEK_DATA / SEEK_HOLE)`:** no pre-content event fires, so a
  partially populated file reports holes for content the HSM has not filled.
- **More `FAN_DENY_ERRNO()` values:** `EACCES` (deny access to content that is
  unavailable, for example while the remote is offline) and `ETIMEDOUT` (a
  fetch timed out, and the client should retry).

### Better fit for an LSM

Deny-only policies that need the operation type, the new values, the caller
credentials or any errno (`EXDEV` for a rename across policy domains,
`EOPNOTSUPP` for hard links) fit LSM hooks better than fanotify. An LSM cannot
wait for userspace, so the policy state must already be in the kernel.
