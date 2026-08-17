---
name: ee-upstream-sync
description: Sync the gwaland/EmptyEpsilon fork from upstream daid/EmptyEpsilon and forward-port upstream master into gwaland-main. Use when asked to sync EE, update from daid, forward-port upstream changes, or prepare an upstream-sync pull request. Do not use for ordinary feature work or cherry-picking one isolated fix.
---

# EmptyEpsilon upstream synchronization

Read `FORK_MAINTENANCE.md` before doing anything. Treat that document as authoritative.

## Repository layout

- `origin` must point to `gwaland/EmptyEpsilon`.
- `upstream` must point to `daid/EmptyEpsilon`.
- `master` is a pristine mirror of `upstream/master`.
- `gwaland-main` is the maintained fork containing custom changes.

## Non-negotiable safety rules

- Require a clean working tree before starting.
- Never rebase `gwaland-main`.
- Never force-push.
- Never add custom commits to `master`.
- Update `master` using fast-forward-only operations.
- Merge `master` into `gwaland-main` using `--no-ff`.
- Do not delete old feature or synchronization branches.
- Do not resolve conflicts using blanket `ours` or `theirs`.
- Do not push directly to `gwaland-main` unless the user explicitly requests the direct workflow.
- Use a pull-request-based synchronization by default.

## Workflow

### 1. Inspect the checkout

Run:

```bash
git status --short --branch
git remote -v
git branch --show-current
```

Stop and report the changed files if the working tree is dirty.

Verify that `origin` references `gwaland/EmptyEpsilon`.

Verify that `upstream` references `daid/EmptyEpsilon`. If it is missing, add it:

```bash
git remote add upstream https://github.com/daid/EmptyEpsilon.git
```

Do not silently replace an existing remote with an unexpected URL. Report it instead.

### 2. Fetch both repositories

```bash
git fetch --prune origin
git fetch --prune upstream
```

Record the previous upstream mirror commit:

```bash
old_master=$(git rev-parse master)
```

### 3. Update the pristine master mirror

```bash
git switch master
git merge --ff-only upstream/master
```

If fast-forwarding is impossible, stop. Explain how `master` diverged instead of resetting or rewriting it.

Record the updated commit:

```bash
new_master=$(git rev-parse master)
```

If `old_master` differs from `new_master`, push the mirror:

```bash
git push origin master
```

### 4. Update the local maintained branch

```bash
git switch gwaland-main
git merge --ff-only origin/gwaland-main
```

If `master` is already contained in `gwaland-main`, report that the fork is already synchronized and do not create a branch or pull request:

```bash
git merge-base --is-ancestor master gwaland-main
```

### 5. Create the synchronization branch

Use:

```bash
sync_branch="sync/upstream-$(date +%F)"
```

If that name already exists, append a numeric suffix rather than deleting or overwriting it.

Create the branch from `gwaland-main`:

```bash
git switch -c "$sync_branch"
```

### 6. Forward-port upstream

```bash
git merge --no-ff master
```

If conflicts occur:

1. List every conflicted file.
2. Examine the upstream change and the fork's custom behavior.
3. Resolve each conflict deliberately.
4. Preserve both behaviors when they are compatible.
5. Never choose all of `ours` or all of `theirs` without file-specific justification.
6. Finish the merge normally after resolving conflicts.

### 7. Validate the result

Always run:

```bash
git status
git diff --check
git log --oneline --graph --decorate -20
```

Inspect the commits between `$old_master` and `$new_master`.

Determine the relevant existing build and test commands from the repository documentation and CI configuration. Run the most appropriate available validation.

Do not claim that a build or test passed unless it was actually run successfully. Clearly report anything that could not be run.

### 8. Push and prepare a draft pull request

Push only the synchronization branch:

```bash
git push -u origin "$sync_branch"
```

Create a draft pull request into `gwaland-main`.

Suggested title:

```text
Forward-port daid/EmptyEpsilon upstream changes
```

The pull request body must include:

- Previous upstream commit: `$old_master`
- New upstream commit: `$new_master`
- Summary of important upstream changes
- Conflicts encountered and how they were resolved
- Build and test results
- Anything requiring manual review

Never merge the pull request automatically unless explicitly instructed.

## Final report

Report:

- The old and new upstream commit IDs
- Whether `origin/master` was updated
- The synchronization branch name
- Conflicts and resolutions
- Validation performed
- Draft pull request URL, if created
