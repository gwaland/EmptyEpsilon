---
name: sync-ee
description: Sync the gwaland/EmptyEpsilon fork from daid/EmptyEpsilon by mirroring upstream master, merging it into a review branch based on gwaland-main, and opening a draft pull request. Use when asked to run /sync-ee, $sync-ee, sync EE, update from daid, or forward-port upstream changes. Do not use for ordinary feature work or isolated cherry-picks.
---

# Sync EmptyEpsilon from upstream

Read the repository-root `FORK_MAINTENANCE.md` before acting and treat it as authoritative. This skill implements its pull-request-based workflow.

## Authorization and stopping conditions

An explicit request to run this synchronization authorizes the workflow's scoped remote writes: updating `origin/master`, pushing the synchronization branch, and opening a draft pull request into `gwaland-main`. Do not ask for another confirmation.

A question about the workflow does not authorize mutations. Never merge the pull request or push directly to `gwaland-main` unless the user separately requests that action.

Stop and report the evidence when:

- the working tree is dirty;
- a configured remote points somewhere unexpected;
- `master` cannot be updated with a fast-forward;
- a conflict's intended resolution is ambiguous; or
- validation fails in a way that makes publishing the review branch unsafe.

## Repository invariants

- `origin` must refer to `gwaland/EmptyEpsilon`.
- `upstream` must refer to `daid/EmptyEpsilon`.
- `master` is a pristine mirror of `upstream/master`; never add custom commits to it.
- `gwaland-main` is the maintained fork branch; never rebase it.
- Merge `master` into the synchronization branch with `--no-ff`.
- Never force-push, delete an existing branch to reuse its name, or resolve conflicts with a blanket `ours` or `theirs` choice.

## Workflow

### 1. Inspect the checkout

Run:

```bash
git status --short --branch
git remote -v
git branch --show-current
```

Require a clean working tree. Verify the remote identities from both fetch and push URLs. If `upstream` is absent, add `https://github.com/daid/EmptyEpsilon.git`; do not silently replace an unexpected existing URL.

### 2. Fetch and update the master mirror

Fetch both repositories:

```bash
git fetch --prune origin
git fetch --prune upstream
```

Switch to `master`, ensure it has not diverged from `upstream/master`, record its current commit as `old_master`, and update it only by fast-forward:

```bash
git switch master
old_master=$(git rev-parse master)
git merge --ff-only upstream/master
new_master=$(git rev-parse master)
```

Verify that `master` and `upstream/master` now identify the same commit. If `old_master` and `new_master` differ, push `master` to `origin`. If they are equal, do not perform an unnecessary push.

### 3. Update the maintained base

Switch to `gwaland-main` and fast-forward it from its remote tracking branch:

```bash
git switch gwaland-main
git merge --ff-only origin/gwaland-main
```

If this cannot fast-forward, stop rather than rewriting or merging unrelated local history.

Check whether `master` is already contained in `gwaland-main`:

```bash
git merge-base --is-ancestor master gwaland-main
```

If it is already contained, report that the fork is synchronized and do not create a branch or pull request.

### 4. Create the review branch and merge

Create `sync/upstream-YYYY-MM-DD` from `gwaland-main`, using the current local date. If that name exists locally or on `origin`, append `-2`, `-3`, and so on until the name is unused. Never delete or overwrite an existing branch.

Merge the mirror branch:

```bash
git merge --no-ff master
```

If conflicts occur, list every conflicted file and inspect the base, upstream change, and fork-specific behavior. Resolve conflicts file-by-file when the intended integration is clear, preserving compatible behavior from both sides, then finish the merge normally. If any resolution is ambiguous, leave the merge in progress and stop with a concise explanation of the decision needed.

### 5. Validate

Always run:

```bash
git status
git diff --check
git log --oneline --graph --decorate -20
```

Review the commits and aggregate diff introduced between `old_master` and `new_master`. Determine appropriate build and test commands from the repository documentation and CI configuration, then run the most relevant available validation. Do not claim a check passed unless it ran successfully. Stop before publishing when a failure plausibly indicates a bad merge; otherwise document environmental limitations clearly.

### 6. Publish for review

Push only the synchronization branch and set its upstream:

```bash
git push -u origin "$sync_branch"
```

Open a draft pull request targeting `gwaland-main`. Use a title such as `Forward-port daid/EmptyEpsilon upstream changes` and include:

- the previous and new upstream commit IDs;
- a concise summary of important upstream changes;
- conflicts encountered and how each was resolved;
- build and test results, including anything not runnable;
- any areas requiring manual review.

Do not merge or mark the pull request ready for review.

## Final report

Report the old and new upstream commit IDs, whether `origin/master` changed, the synchronization branch, conflict resolutions, validation results, and the draft pull request URL. If the fork was already synchronized or the workflow stopped, state that outcome and the exact reason.
