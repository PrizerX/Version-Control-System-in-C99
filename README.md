# prk (Mini Version Control System in C99)

## 1) Project Overview

`prk` is a terminal-based, Git-like mini version control system written strictly in C (C99).
It demonstrates the core VCS ideas using only files and folders:

- repository initialization
- content hashing
- object storage
- staging/index tracking
- commit history with parent links
- checkout by commit id
- commit-to-commit diff

No external libraries are used.

## 2) Features Implemented

- `init`
  - creates `.mygit/`, `.mygit/objects/`, `.mygit/commits/`, `.mygit/refs/`
  - creates `.mygit/HEAD`
  - creates `.mygit/index.txt`
- `add <filename>`
  - validates file exists
  - enqueues filename into staging queue
  - persists queue in `.mygit/queue.txt`
- `commit "<message>"`
  - dequeues staged filenames one by one
  - processes each file into objects + index
  - creates commit file `.mygit/commits/<commit_id>.txt`
  - stores commit id, message, parent, timestamp, tracked file list
  - updates `HEAD`
- `undo`
  - pops latest commit from commit stack
  - checks out to previous commit
  - if no previous commit exists, sets `HEAD` to `none`
- `log`
  - walks parents from `HEAD` to `none`
  - prints newest to oldest
- `checkout <commit_id>`
  - restores tracked files for that commit from objects
  - updates `HEAD` to checked-out commit
- `diff <commit1> <commit2>`
  - compares tracked files between two commits
  - prints line-by-line differences

## 3) Folder Structure Explanation

```
.
|-- main.c
|-- repo.h / repo.c
|-- commit.h / commit.c
|-- hash.h / hash.c
|-- diff.h / diff.c
|-- utils.h / utils.c
`-- README.md
```

Runtime repository layout after `prk init`:

```
.mygit/
|-- HEAD
|-- index.txt
|-- queue.txt
|-- objects/
|-- commits/
`-- refs/
```

## 4) How Each Command Works

### `prk init`

1. Ensures `.mygit` and required subfolders exist.
2. Creates `HEAD` with value `none` if missing.
3. Creates empty `index.txt` if missing.

### `prk add <filename>`

1. Validates file content can be read from working directory.
2. Enqueues filename in staging queue.
3. Saves queue state to `.mygit/queue.txt`.

### `prk commit "<message>"`

1. Loads queue from `.mygit/queue.txt`.
2. Dequeues and processes each queued file (object storage + index update).
3. Reads all currently tracked entries from index.
4. Reads parent from `HEAD`.
5. Generates commit id from message + parent + timestamp + tracked entries.
6. Writes commit file:
   - `commit_id:<id>`
   - `parent:<parent or none>`
   - `timestamp:<epoch>`
   - `message:<text>`
   - `files:` then `filename|hash` lines
7. Updates `HEAD` with new commit id.

### `prk undo`

1. Builds commit stack from current `HEAD` chain.
2. Pops latest commit hash.
3. Checks out previous commit hash.
4. If no previous commit remains, updates `HEAD` to `none`.

### `prk log`

1. Reads `HEAD`.
2. Loads that commit file.
3. Prints details.
4. Moves to `parent` and repeats until `none`.

### `prk checkout <commit_id>`

1. Loads commit file by id.
2. For each tracked file entry, finds object file by hash.
3. Copies object content into working directory file path.
4. Updates `HEAD` to checked-out commit id.

### `prk diff <commit1> <commit2>`

1. Loads both commits.
2. Builds union of filenames tracked by either commit.
3. For each changed file hash, reads both object contents.
4. Prints per-line `- old` and `+ new` changes.

## 5) Compilation and Usage Instructions

Compile:

```bash
gcc *.c -o prk
```

Example usage:

```bash
./prk init
./prk add notes.txt
./prk commit "first commit"
./prk undo
./prk log
./prk checkout <commit_id>
./prk diff <commit1> <commit2>
```

On Windows (PowerShell), run:

```powershell
./prk.exe init
```

## 6) Line-by-Line Explanation (Beginner-Friendly)

### A) `main.c` (CLI parsing and routing)

- Includes headers for modules:
  - `repo.h` for repository commands
  - `commit.h` for commit/log/checkout
  - `diff.h` for commit comparison
- `print_usage()`:
  - prints all available commands and expected arguments.
- `main(int argc, char *argv[])`:
  - checks whether at least one command is provided.
  - compares `argv[1]` to supported commands using `strcmp`.
  - validates argument count per command.
  - calls the corresponding module function.
  - returns exit code `0` on success, `1` on failure.

Why this is useful:
- keeps command parsing centralized.
- keeps business logic in dedicated modules.

### B) Commit logic (`commit.c`)

- `Commit` struct stores:
  - `id`
  - `parent`
  - `message`
  - `timestamp`
  - linked list of tracked files (`FileEntry`)

Important functions:

1. `create_commit(...)`
   - verifies repository exists and message is non-empty.
   - loads staged/index entries.
   - reads parent from `HEAD`.
   - builds a deterministic input string with message/parent/time/files.
   - hashes this string to produce `commit.id`.
   - writes commit metadata + tracked file list to a commit file.
   - updates `HEAD`.

2. `load_commit(...)`
   - opens commit file by id.
   - parses metadata lines (`parent:`, `timestamp:`, `message:`).
   - parses file lines (`filename|hash`) into linked list.

3. `print_commit_log()`
   - starts from `HEAD`.
   - loads and prints each commit.
   - follows `parent` until root (`none`).

4. `checkout_commit(...)`
   - loads target commit.
   - restores each tracked file from its object hash.
   - updates `HEAD`.

Why this design helps:
- parent links form a simple commit chain.
- commit files are human-readable and easy to debug.

### C) Hashing logic (`hash.c`)

- `hash_string(...)`:
  - uses the classic DJB2-style rolling hash pattern.
  - starts from seed `5381`.
  - for every character: `hash = hash * 33 + c`.
- `hash_buffer_hex(...)`:
  - same algorithm over bytes.
  - writes result as hexadecimal text using `snprintf`.

Why this is acceptable here:
- deterministic and simple.
- no external cryptographic library required.

Note:
- this is not cryptographically secure (fine for this educational mini VCS).

### D) File storage system (`repo.c` + `utils.c`)

Core storage pieces:

1. Objects
   - every added file content is saved to:
     - `.mygit/objects/<hash>.txt`
   - this allows content-addressable lookup.

2. Index (staging/tracking)
   - `.mygit/index.txt` has one line per tracked file:
     - `filename|hash`
   - `repo_add_file(...)` updates existing entries or appends new ones.

3. HEAD
   - `.mygit/HEAD` stores current/latest commit id (or `none`).

4. Commit files
   - `.mygit/commits/<commit_id>.txt` stores metadata and tracked snapshot.

Support helpers in `utils.c`:

- `path_exists(...)` checks path existence.
- `ensure_dir(...)` creates missing folder.
- `read_text_file(...)` reads full file into heap buffer.
- `write_text_file(...)` writes full file content.
- `copy_file(...)` copies bytes (used by checkout).
- linked-list helpers:
  - append, find, free file entries.

Why linked lists were used:
- requirement asks for linked lists in file tracking.
- file count is usually small in this educational project.
- easy insertion and traversal.

---

## Notes

- This project is intentionally simple to teach VCS internals.
- It does not implement advanced Git features such as branching/merging/conflict resolution.
- All modules are fully implemented and compile together under C99.
