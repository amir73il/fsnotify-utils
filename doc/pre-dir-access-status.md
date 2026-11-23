# fanotify pre-dir-content (FAN_PRE_DIR_ACCESS) — WIP status

Status as of 2026-11-24: `fan_pre_dir_access` on v7.4-rc3 (12 commits on
`fan_pre_content_fid`, head `72b1feb0261b`, pushed).
Commits are referred to by subject, since the branch is rebased often.
Scratch notes, not for submission.

- Kernel branch: `fan_pre_dir_access` (github: amir73il/linux), on top of
  `fan_pre_content_fid`, which is on top of `fsnotify-fixes`
- LTP branch: `fan_pre_dir_access` (~/src/ltp, head `aa09c9656e44`, pushed), tests
  `fanotify14` (invalid flags) and `fanotify28` (events)
- Background: ~/src/fsnotify-utils/wiki/Hierarchical-Storage-Management-API.mediawiki
  (`FAN_PATH_ACCESS` / `LOOKUP_PERM` in the old httpdirfs `fan_hsm_poc`)

## Already upstream (v6.14 .. v7.4-rc3)

- `FAN_PRE_ACCESS` on regular files only (FAN_ONDIR rejected), with range info
  (`FAN_EVENT_INFO_TYPE_RANGE`, 64-bit offset/count)
- `FAN_CLASS_PRE_CONTENT`, `FAN_DENY_ERRNO()`, filesystems opt in with
  `SB_I_ALLOW_HSM` (ext4/xfs/btrfs)
- HSM mode chosen once at open: `FMODE_NONOTIFY | FMODE_NONOTIFY_PERM` ==
  `FMODE_FSNOTIFY_HSM()`
- Hooks: `rw_verify_area()`, `remap_verify_area()`, `vfs_fallocate()`, mmap
  (`fsnotify_mmap_perm()`), truncate. Page-fault hooks were reverted in 6.14-rc7.

## Posting history

| Date       | What                                                                  | Message-ID |
|------------|-----------------------------------------------------------------------|------------|
| 2025-06-04 | [RFC PATCH v2 0/3] fanotify HSM events for directories (FAN_PRE_ACCESS+FAN_ONDIR, FAN_PATH_ACCESS) | `20250604160918.2170961-1-amir73il@gmail.com` |
| 2025-06-10 | Jan: merge them into one FAN_PRE_DIR_ACCESS (name on lookup, none on readdir) | `e2rcmelzasy6q4vgggukdjb2s2qkczcgapknmnjb33advglc6y@jvi3haw7irxy` |
| 2025-07-04 | Agreed: internal lookups always allowed, negative result -> drop + ENOENT | `CAOQ4uxhttUt6makW6GZGfBb=rat+gH9QQmparX3cexDJwzhVMw@mail.gmail.com` |
| 2025-07-07 | [PATCH 0/2] More fsnotify hook optimizations (merged as 0d4c4d4ea443) | `20250707170704.303772-1-amir73il@gmail.com` |
| 2025-07-08 | Branch link only: github.com/amir73il/linux/commits/fan_pre_dir_access/ | `CAOQ4uxiZ=UH+Dp974pecpUq1V8OwJBrzb-gtDRDDEGXL=0nVpA@mail.gmail.com` |
| 2025-07-08 | Jan on FAN_REPORT_RESPONSE_ID: only require fid mode, questions FAN_CLASS_PRE_CONTENT_FID | `pcyd4ejepc6akgw3uq2bxuf2e255zhirbpfxxe463zj2m7iyfl@6bgetglt74ei` |
| 2025-07-09 | Design write-up in prose (handle once, DCACHE_HSM_ONCE, nfsd hook)     | `CAOQ4uxjNsY4jyeBFGY9Pu=qV2k_oMjC0LM6k9uk0X_6mQH9OoA@mail.gmail.com` |
| 2025-07-09..11 | Ibrahim's "response identifier" v2-v4 carried patches 1-2 of `fan_pre_content_fid` | `20250711183101.4074140-1-ibrahimjirdeh@meta.com` |
| 2025-07-23 | Asked Jan for a look; plan: take response_id, make events fd-less     | `CAOQ4uxiJU3PjM3fcUgJMSb0AU+ekXUMbUwyns08Q08zhwej9KQ@mail.gmail.com` |

The `FS_PRE_DIR_ACCESS` patches themselves were never posted. Jan did not reply
after 2025-07-04.

## API

- **Group:** `FAN_CLASS_PRE_CONTENT_FID` = `FAN_CLASS_PRE_CONTENT |
  FAN_REPORT_DFID_NAME_TARGET`, with new top priority
  `FSNOTIFY_PRIO_PRE_CONTENT_FID`.
  - Only pre-content events are allowed.
  - No event flags are allowed; `FAN_EVENT_ON_CHILD` is implied on a
    directory inode mark.
  - Events carry fid info **and** `event->fd`.
- **`FAN_PRE_DIR_ACCESS` (0x00200000)**, reported on the directory itself,
  never on its parent:
  - readdir (`iterate_dir()` -> `fsnotify_file_perm()`): dir fid, no name,
    meaning "populate the whole directory"
  - lookup, on a dcache miss only: dir fid + name, so the HSM can populate a
    single entry. Sent from path walk (`lookup_slow_notify()`), from the last
    component of `open()` (`open_last_lookups()` before `lookup_open()`) and
    from the last component of create / remove / rename syscalls
- **`FAN_PRE_ACCESS` with fid:** DFID+name and child fid. The HSM can open
  by handle through a bind mount that has an ignore mark.

## Design

- **Handle once:**
  - An allowed readdir event sets `DCACHE_HSM_ONCE` on the dentry, which
    suppresses all further pre-dir events on that directory until the
    dentry is evicted.
  - A lookup event does not set the flag, since the HSM may have populated
    only one entry.
  - Dentries that were cached before the mark was added never generate
    events.
- **Cheap when unwatched:**
  - Open-time `FMODE_*` selection now also covers directories.
  - Path walk stops calling the hook after `-EOPNOTSUPP` until it crosses a
    mount, through `nd_lookup_notify()` and `ND_NONOTIFY`, in "fsnotify:
    optimize out multiple pre-dir-content events during path walk".
  - `fsnotify()` returns >0 for a pre-dir-content event that was handled by
    userspace, in "fsnotify: return positive value if pre-dir-content event
    was handled". Only readdir uses it, to set
    `DCACHE_HSM_ONCE`.
  - In namei, `__lookup_notify()` maps the hook result to < 0 denied, 0
    allowed (handled or not) and > 0 no pre-content watchers on this sb, so
    callers only check < 0 and path walk uses > 0 to set `ND_NONOTIFY`.
- **Last component of create / remove / rename syscalls**
  (`filename_create()`, `filename_rmdir()`, `filename_unlinkat()`,
  `filename_renameat2()` for both parents): `lookup_last_notify()` sends the
  blocking lookup event with the name before `mnt_want_write()` and the
  parent lock, then passes `LOOKUP_NONOTIFY` to the locked lookup.
- **Last component of open()**, with and without `O_CREAT`:
  `open_last_lookups()` calls `nd_lookup_notify()` after a
  `lookup_fast_for_open()` miss and `try_to_unlazy()`, before `lookup_open()`
  takes freeze protection and the parent lock, in "fsnotify: generate
  pre-dir-content event before create/remove/rename".
  `lookup_open()` does not use `lookup_perm_negative()`, so an allowed
  `creat()` of a new name creates it.
- **Internal VFS users** (ovl, nfsd, cachefiles) never block:
  - The hook contract was extended: `fsnotify_lookup_perm()` may be called
    with a NULL path from a context that is not safe for blocking.
    With a NULL path, no event is sent; fanotify only returns `-ENOENT`
    if the lookup should have been handled in userspace.
  - `lookup_perm_negative()` does this check on a negative result of a fresh
    `->lookup()`, and on `-ENOENT` drops the negative dentry
    (`lookup_drop_negative()`) and fails the lookup.
  - It is called from `__lookup_slow()` (`lookup_one*()` helpers) and from
    `lookup_one_qstr_excl()` (`start_creating()` and friends).
  - Path walk and the syscalls above pass `LOOKUP_NONOTIFY` to skip it.
- **nfsd (optional, 2 patches):** a readdir-style hook once per `svc_fh`, from
  `fh_verify(NFSD_MAY_EXEC)` and from `fh_want_write()` before
  `mnt_want_write()`. `-EOPNOTSUPP` (no pre-dir-content watchers) means
  allowed. The HSM's errno is passed to the client, except `EBUSY`, which
  `nfserrno()` does not map: it is reported as `EACCES`
  (`NFS4ERR_ACCESS` / `NFS3ERR_ACCES`), the way `nfsd_link()`,
  `nfsd_rename()` and `nfsd_unlink()` map `EBUSY` on directories. An HSM that
  wants the client to retry denies with `EAGAIN` (`NFS3ERR_JUKEBOX` /
  `NFS4ERR_DELAY`). A blanket `EBUSY` mapping in `nfserrno()` was rejected
  by Chuck and Neil in 2025 (`20250121103954.415462-1-amir73il@gmail.com`),
  which asked for per-call-site mappings.

## Test status

On 2026-11-24, kernel `ca0f070c69f1` plus the cleanups that are now folded
into the branch (build #3084; the pushed head differs only by one blank line)
in the kvm-xfstests VM: an nfsd loopback check (NFSv3 and v4.2 on an xfs
export) with a minimal `FAN_PRE_DIR_ACCESS` sb-mark listener. Without a
listener, ls / cat / touch / mkdir / rm in a directory work; with an allowing
listener nfsd sends events (no name); a first event denied with EIO gives the
client EIO and the retry works; EBUSY gives EACCES ("Permission denied") and
the retry works; EAGAIN makes the client retry on its own and succeed after
5s. No "non-standard errno" warning in dmesg. With build #3083 (the same
cleanups, EBUSY still mapped to EAGAIN): fanotify14 484 passed, 0 failed;
fanotify28 unchanged from 2026-11-23.

On 2026-11-23, kernel `ca0f070c69f1` (build #3082) in the kvm-xfstests VM,
LTP `aa09c9656e44`, all filesystems: fanotify28 variant 0 passes. The
overlayfs variants 1 and 2 fail with 25 TFAILs each on ext3, ext4, xfs and
btrfs, all from the three overlayfs gaps in Known issues (mmap event, create
in a lower dir watched by a parent mark, execve ETXTBSY); there is no TBROK
or TWARN. Overall 1524 passed, 200 failed, 41 skipped (the mount mark cases
in the overlayfs variants, and ext2, vfat and tmpfs). fanotify14 was not
rerun.

Earlier, on 2026-11-22, kernel `bc8cb6919a59` (build #3080) in the kvm-xfstests VM,
LTP `a622f0f944bb`, all filesystems, `-i 2`: fanotify14 PASS (968 passed, 42
skipped) and fanotify28 PASS (2032 passed, 3 skipped). The fanotify28 skips
are ext2, vfat and tmpfs, which don't support pre-content events. dmesg is
clean. fanotify28 is not in the VM's `runltp.sh` list, so it runs by hand.
fstests overlay has not been run against this code yet.

## fanotify14 coverage

- fid rules, in "fanotify14: Test invalid flags with pre-content events
  with fid info": `FAN_CLASS_PRE_CONTENT` with a partial
  fid mode, and `FAN_OPEN_PERM`, `FAN_ONDIR` or `FAN_EVENT_ON_CHILD` with
  `FAN_CLASS_PRE_CONTENT_FID`, all `EINVAL`
- directory rules, in "fanotify14: Test invalid flags with pre-content
  events on directories": `FAN_PRE_DIR_ACCESS` with
  `FAN_CLASS_PRE_CONTENT` (`EINVAL`), and on a non-dir inode mark
  (`ENOTDIR`)
- a single `pre_content_fid_unsupported` check skips these on old kernels
  and on filesystems without pre-content support

## fanotify28 coverage

Twelve test cases over inode, mount, sb and parent marks, in three test
variants. Each event in the table carries the path of its expected name
(`NULL` for readdir), and the expected name is the last path component. The
child consumes the event of each operation by mask and name
(`consume_event()`). A mount cycle between cases resets `DCACHE_HSM_ONCE` and
removes the populated and created entries. The test runs in
`mntpoint/testdir`; mount and sb marks are set via the mount root and parent
marks on `testdir`, so setting a mark does not cache dentries in `testdir`.

- file `FAN_PRE_ACCESS` with fid, in "fanotify28: Add test for pre-content
  events with fid info": ALLOW / DENY / DENY_ERRNO(EIO, EBUSY)
  on mmap / write / read / execve, with the file names checked. Only an
  allowed `FAN_PRE_ACCESS` event at the end of the set may repeat (execve).
- open by handle via an ignored bind mount, and no events from the event fd,
  in "fanotify28: open file by file handle"
- dir events, in "fanotify28: Add test cases for pre-content events on
  directories":
  - a lookup of `testdir/enoent_N` with the name checked. With a mount or sb
    mark, the same path walk reports `testdir` (in the mount root) and then
    `enoent_N` (in `testdir`), so a handled event must not stop the hook for
    the rest of the walk
  - `FAN_ALLOW_ENOENT` is a test marker that tells the listener to populate
    the name and allow, so the lookup succeeds. The listener opens the
    event's dir by its fid through the ignored bind mount and creates the
    entry there (real populate from inside the event)
  - readdir without a name, then no more dir events (handle once)
  - denied lookup and readdir, retried until allowed (a denied event is not
    handled, so it is generated again)
- create / rename / remove, in "fanotify28: Test pre-content events on
  create, rename and remove": no events after an allowed readdir;
  without readdir, `mknod()` (denied once, then retried), `rename()` (old and
  new name) and `unlink()` report events with the entry names, and so do the
  `open()` of the existing `fname` and the `execve()` lookup that follow
- open, in "fanotify28: Test pre-content events on open": after
  the lookup, `open(newent_N)` is denied (EPERM) and `creat(newent_N)` is
  allowed and creates the entry
- overlayfs variants, in "fanotify28: Add overlayfs test variants",
  prepared by "fanotify28: Prepare for different watched and accessed test
  dirs", "fanotify28: Reset the test dir in mount cycle" and "fanotify28:
  Prepare for test variants with unexpected events".
  Variant 1 watches the lower layer and variant 2 the upper layer,
  and both access the test files via overlayfs:
  - no lookup events via overlayfs (internal user); lookups of missing
    names, including `FAN_ALLOW_ENOENT`, fail with ENOENT
  - creates fail with ENOENT until an allowed readdir populates the watched
    layer dir; with an sb mark in variant 1, the copy up fails with ENOENT
    also after readdir
  - readdir, mmap, read and execve send the same events on the real files as
    on the base fs; in variant 1 the file is opened read only (no copy up)
    and the write is replaced by a read of another range
  - mount marks are not tested (overlayfs uses private mounts)

Child failures that depend on events report TFAIL and exit, not TBROK, so
`cleanup()` still unmounts the bind mount.

**Not covered:**

- [ ] the negative-lookup path for nfsd

## Open questions (from the list)

1. **No review of the branch.** It was sent only as a link and Jan never
   replied.
2. **Response ID vs event->fd** (Jan, 2025-07-08):
   - `FAN_REPORT_RESPONSE_ID` should require only fid mode.
   - Is a separate `FAN_CLASS_PRE_CONTENT_FID` and priority needed at all?
   - Are fd-based events useful enough for pre-dir events?
   - The plan from 2025-07-23 was to take Ibrahim's response_id and go
     fd-less. Not upstream yet, and the branch and test still use
     `event->fd`. **This decides the uapi, so settle it before posting.**
3. **Who modifies directories?** (Jan, 2025-07-04) Is dir sync one-way like
   files? Partly answered by the code now: create / remove / rename
   syscalls and `open()` / `open(O_CREAT)` ask the HSM first, and internal
   users fail with ENOENT in an unpopulated directory. Still missing: the
   proposed handling of entries created behind the HSM's back (the HSM
   detects conflicts at populate time and returns ESTALE), which is neither
   implemented nor documented.
4. **What happens to pending events when the HSM dies?** When the HSM group
   is released (e.g. the HSM is SIGKILLed) while a reader is blocked in
   `FAN_PRE_ACCESS`, the pending event is allowed, so the reader reads holes
   instead of data (seen with the httpdirfs POC). For an HSM, failing pending
   pre-content events (e.g. with EIO) may be the safer default.
5. **Path-less lookups** that send real events with a dir fid (ovl, nfsd,
   ksmbd) instead of failing with ENOENT: deferred, and the API must not
   rule it out. Fid-only/response-id events would make it possible.
6. **Implied `FAN_EVENT_ON_CHILD` in the fid class.** "fanotify: allow
   pre-content events with fid info" forbids event flags in
   `FAN_CLASS_PRE_CONTENT_FID` and implies `FAN_EVENT_ON_CHILD` for
   `FAN_PRE_ACCESS` on a directory inode mark. Upstream (v6.14)
   `FAN_CLASS_PRE_CONTENT` requires an explicit `FAN_EVENT_ON_CHILD`; without
   it the mark is accepted and reports nothing. So the same `fanotify_mark()`
   call on a directory means "nothing" in one class and "the children" in
   the other, and the explicit flag is EINVAL in the fid class. Options:
   (1) allow `FAN_EVENT_ON_CHILD` in the fid class (forbid only `FAN_ONDIR`,
   which `FAN_PRE_DIR_ACCESS` does not need) and require it, or reject
   `FAN_PRE_ACCESS` on a directory inode mark without it; (2) keep it implied
   and justify it in the man page and cover letter (in the new class event
   flags carry no meaning, the object type decides). Raise in the RFC.

## Known issues to fix

- [ ] **The range info record can be misaligned with fid info (uapi).** Info
      records are padded to `FANOTIFY_EVENT_ALIGN` (4), and
      `struct fanotify_event_info_range` has `__u64` members at offset 8 of
      the record. Upstream (v6.14) the range record follows the 24 byte
      metadata or the 8 byte pidfd record, so it is always 8-byte aligned.
      With `FAN_CLASS_PRE_CONTENT_FID`, `copy_info_records_to_user()` copies
      the fid records first, and a fid record (hdr + fsid + handle + name,
      rounded to 4) often ends at 4 mod 8, so the range record is misaligned.
      Userspace that reads it in place gets undefined behavior (found by UBSan
      in the httpdirfs POC; it may fault on strict-alignment archs).
      Options: (1) pad the record before the range record so the range record
      starts 8-byte aligned, which is compatible because parsers step by
      `hdr.len`; (2) report the range record first, which breaks parsers that
      assume the fid record comes first (e.g. fanotify28); (3) document that
      records must be copied before use. Misalignment is not to be accepted
      as uapi, so (3) is out.
      **Planned fix (1):** in `FAN_CLASS_PRE_CONTENT_FID` groups, pad every fid
      info record to 8 bytes instead of `FANOTIFY_EVENT_ALIGN` (4). The pidfd
      (8) and range (24) records are multiples of 8, so the range record then
      always starts 8-byte aligned. The class is new, so nothing existing
      changes, and the extra zero padding is invisible to parsers that step by
      `hdr.len`. Code: `fanotify_fid_info_len()` takes the group's alignment
      (via `fanotify_dir_name_info_len()` / `fanotify_event_len()`), and the
      padding `WARN_ON_ONCE()` in `copy_fid_info_to_user()` checks against it.
      It belongs in "fanotify: allow pre-content events with fid info",
      where fid and range records first meet in one event.
      fanotify28 does not parse the range info record yet; the regression
      check needs to add range parsing and check that the record offset in
      the event is a multiple of 8. The fsnotify-utils `fanotify_example`
      (a895b14 "fanotify_example: Test FAN_PRE_DIR_ACCESS") and the httpdirfs
      POC (5e9590b "fanotify: Populate directories with FAN_PRE_DIR_ACCESS
      events") read the record in place and rely on this fix; until then,
      UBSan reports the misaligned access in the POC.
- [x] **`open()` did not send the lookup event for its last component**,
      fixed 2026-11-22 in "fsnotify: generate pre-dir-content event before
      create/remove/rename", with the `nd_lookup_notify()` helper in
      "fsnotify: optimize out multiple pre-dir-content events during path
      walk". On a dcache miss,
      `open_last_lookups()` went straight to `lookup_open()`, so
      `open(O_RDONLY)` in an unpopulated directory returned ENOENT without
      asking the HSM, and `open(O_CREAT)` created locally even if the name
      existed remotely. Found by the fanotify28 open cases, which now pass.
- [x] **Regression from the v7.4 rebase**, fixed 2026-11-21 in "fsnotify:
      fail some negative lookup results for internal vfs users", plus the new
      "fsnotify: generate pre-dir-content event before create/remove/rename".
      Upstream moved overlayfs creates to
      `start_creating()` -> `start_dirop()` -> `lookup_one_qstr_excl()`, which
      bypassed the negative-lookup check in `__lookup_slow()`. The check now
      also runs in `lookup_one_qstr_excl()`, and the create / remove / rename
      syscalls send the event first and pass `LOOKUP_NONOTIFY`. The lockdep
      assertion in `fsnotify_lookup_perm()` now allows a NULL path. The
      syscall side is tested by fanotify28; the overlayfs side is not.
- [x] **nfsd: permission bypass**, fixed 2026-11-19 and squashed into "nfsd:
      add pre-dir-content fsnotify hooks". In
      `__fh_verify()`, a `nfsd_permission()` error was overwritten by the `0`
      from `nfsd_pre_dir_content()` for every `NFSD_MAY_EXEC` request, so a
      client could look up names in a directory it has no search permission
      on. It came with the original patch, not the rebase, and was never
      posted. The hook now runs only
      `if (!error && (access & NFSD_MAY_EXEC))`, which also keeps
      unauthorized lookups from making the HSM populate.
- [x] **`FAN_PRE_DIR_ACCESS` was allowed for `FAN_CLASS_PRE_CONTENT` without
      fid**, fixed 2026-11-21 in "fsnotify: prepare for pre-dir-content
      events". The commit message said the event is for
      `FAN_CLASS_PRE_CONTENT_FID` only, but `do_fanotify_mark()` had no check
      for `FSNOTIFY_PRIO_PRE_CONTENT`. Found by the new fanotify14 case, which
      now passes.
- [ ] **mmap via overlayfs (and other backing files) sends no pre-content
      event on the real file.** `fsnotify_mmap_perm()` is only called from
      `vm_mmap_pgoff()`, on the user file (the overlayfs file, whose sb is
      not watched). `backing_file_mmap()` switches `vma->vm_file` to the
      backing file and calls `vfs_mmap()`, but does not call
      `fsnotify_mmap_perm()` on it, so an HSM watching the real file gets no
      `FAN_PRE_ACCESS` for the mmap. Faults on the mapping are still notified
      on the real file, and read/write via `backing_file_read_iter()` /
      `backing_file_write_iter()` send events on the real file through
      `vfs_iocb_iter_read()` / `vfs_iocb_iter_write()`. Found by the fanotify28
      overlayfs variants (2026-11-23): on a pure upper file, write and read
      report events, mmap does not.
- [ ] **Create via overlayfs in a lower dir watched by a parent mark
      succeeds.** Expected ENOENT: the create looks up the new name in the
      watched, unpopulated lower dir, which is a negative lookup by an
      internal user. Found by the fanotify28 overlayfs variant 1 (#7, #8,
      #10, 2026-11-23); `open()` of the same name just before gets ENOENT as
      expected. Cause not analyzed yet; a negative dentry cached in the lower
      dir by the earlier lookup is a suspect.
- [ ] **execve via overlayfs of a file open for write fails with ETXTBSY**,
      although the real file has pre-content watches. On the base fs, a
      watched file open for write can be executed, so that the HSM can fill
      it while it is executed. Found by the fanotify28 overlayfs variant 2
      (#1, pure upper file, 2026-11-23). Cause not analyzed yet; the exec
      write denial seems to check the overlayfs inode, which is not watched.
- [ ] **HSM below overlayfs cannot populate for overlayfs (limitation).**
      Overlayfs looks up names in its layers as an internal user (no path),
      so with "fsnotify: fail some negative lookup results for internal vfs
      users" a negative lookup in a watched, unpopulated layer
      dir fails with ENOENT instead of sending an event to the HSM. Through
      overlayfs (fanotify28 overlayfs variants, 2026-11-23):
      - no lookup events: lookups and opens of a missing name get ENOENT, and
        a lookup that the HSM would populate stays ENOENT
      - creates in an unpopulated upper dir that the HSM watches (parent or
        sb mark) fail with ENOENT, until a readdir through overlayfs is
        allowed and marks the upper dir populated
      - with an sb mark on the layers' fs, copy-up fails with ENOENT, because
        it looks up the dir names in the unpopulated upper dirs, which no
        readdir through overlayfs populates (readdir of a lower-only dir
        reads only the lower dir)
      - readdir through overlayfs sends a readdir event for each watched
        layer dir that is read
      A fix needs path-less events (open question 5), so the HSM gets events
      from overlayfs lookups.
- [x] **nfsd: every directory access failed with EPERM without an HSM**,
      fixed 2026-11-24 in "nfsd: add pre-dir-content fsnotify hooks".
      `nfsd_pre_dir_content()` mapped every error to `-EPERM`, but
      `fsnotify_lookup_perm()` returns `-EOPNOTSUPP` when the sb has no
      pre-dir-content watchers (since "fsnotify: optimize out multiple
      pre-dir-content events during path walk"), so `NFSD_MAY_EXEC` and
      `fh_want_write()` on a directory failed on every export. Found by
      review; the nfsd loopback check (Test status) covers it.
- [x] **nfsd: the HSM's errno is not passed through**, fixed 2026-11-24 in
      the same commit: the errno is passed through and `EBUSY` is reported
      as `EACCES` (see Design).
- [x] **nfsd:** `fh_put()` did not clear `fh_got_content`, fixed 2026-11-24.
- [ ] **No way to re-arm a directory:** `DCACHE_HSM_ONCE` lives until the
      dentry is evicted, and an HSM that evicts a directory's content cannot
      ask for events again. It also survives an HSM restart: an entry deleted
      from the HSM's local copy while the HSM was down is not repopulated
      until the dentry is evicted (e.g. remount), because no readdir event
      comes (seen with the httpdirfs POC).
- [x] `lookup_one_positive_unlocked()` gained `drop_negative`, but
      `lookup_one_positive_killable()` did not. That is intended: its only
      caller is ovl readdir, which looks up names it just read. The porting.rst
      note was added 2026-11-24 in "fs: factor out helper
      lookup_drop_negative()".
- [x] Typos, fixed 2026-11-24: in commit messages, "usersapce" and
      "travesing" ("fanotify: generate pre-dir-content event with name info on
      lookup"), "memeber" and others ("fanotify: add support for a variable
      length permission event"), "memebr" ("nfsd: un-inline fh_want_write()")
      and "the the" ("fanotify: allow pre-content events with fid info");
      in comments, "with with" in `fanotify.h` and "Singal" in `fsnotify.h`.
      "evnets" in `fanotify_user.c` is upstream (v6.14), not from this series.
- [x] Indentation in `do_fanotify_mark()` ("fanotify: allow pre-content
      events with fid info") and a double blank line in `fsnotify.c`,
      fixed 2026-11-24. checkpatch `--strict` is clean on all 14 commits,
      except the item below.
- [x] Comment on the `smp_load_acquire()` in `lookup_perm_negative()` (pairs
      with `smp_store_release()` in `__d_set_inode_and_type()`), added
      2026-11-24. checkpatch still reports "memory barrier without comment",
      because the comment is above the `if`, not on the line before the
      barrier (false positive).
- [x] The kernel-doc of `fsnotify_lookup_perm()` now documents its arguments
      (NULL path for internal users) and all results: `-EOPNOTSUPP`, >0, 0,
      <0, and `-ENOENT` with a NULL path. Fixed 2026-11-24.
- [ ] Latent in fanotify28: `SAFE_MMAP()` and `SAFE_OPEN(fname)` in the child
      depend on event responses. They work because the tables allow those
      events, but a denied or misordered event would TBROK and leak the bind
      mount. (The listener no longer populates on an unexpected event, which
      removed the TBROK seen while measuring the overlayfs variants.)
- [ ] **Man page.** Needs updating for the new API: `FAN_PRE_DIR_ACCESS`
      (readdir without a name, lookup with a name, handle once via
      `DCACHE_HSM_ONCE`, create/remove/rename/open coverage),
      `FAN_CLASS_PRE_CONTENT_FID` (only pre-content events, no event flags,
      implied `FAN_EVENT_ON_CHILD` on a directory inode mark, events carry fid
      info and `event->fd`), ENOENT for internal users in an unpopulated
      directory, and the range info record alignment once fixed. Branch
      `fan_pre_dir_access` in ~/src/man-pages has 33549c45d ("fanotify:
      Document FAN_PRE_DIR_ACCESS events"), a draft for the current design
      to be used as a reference for the real patch.
- [ ] Run fstests overlay against the new code (see Test status for LTP).

## Next steps

1. Decide response_id / fd-less and whether a separate class is needed;
   rebase onto that.
2. Fix the overlayfs gaps found by the fanotify28 overlayfs variants. Run
   fstests overlay.
3. Post RFC v3, with a cover letter that answers Jan's 2025-07-04 and
   2025-07-08 questions.
