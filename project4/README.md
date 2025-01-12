# Distributed File System

## Project Overview
This project involves creating a **Distributed File System (DFS)**, a server-side application that enables multiple clients to access and manipulate shared files concurrently over a network. The distributed file system provides operations for reading, writing, and deleting files using an HTTP/REST API. It draws inspiration from Amazon’s S3, a popular storage system.

Key learning goals include:
- Understanding on-disk structures for file systems.
- Exploring the internals of file systems.
- Learning about distributed storage systems.

The project is implemented in three parts:
1. **Reading on-disk storage**: Using command-line utilities.
2. **Creating a local file system**: Organizing disk blocks into a hierarchical structure.
3. **Building a distributed file system**: Extending the local file system to provide network-accessible APIs.

### Features
- Persistent storage with crash consistency.
- Directory and file management.
- HTTP/REST API for client interactions.
- Error handling to ensure filesystem integrity.

---

## HTTP/REST API Overview
The DFS uses HTTP methods to interact with files and directories:

### Files
- **Create/Update**: Use `PUT` with the file path and contents in the request body.
- **Read**: Use `GET` with the file path to retrieve its contents.
- **Delete**: Use `DELETE` with the file path.

### Directories
- **Create**: Created implicitly during file creation.
- **List Contents**: Use `GET` with the directory path to list entries, sorted alphabetically. Entries for subdirectories include a trailing `/`.
- **Delete**: Use `DELETE` with the directory path. Deletion of non-empty directories is an error.

### Example Commands (Using `cURL`):
```bash
# Create or update a file
curl -X PUT -d "file contents" http://localhost:8080/ds3/a/b/c.txt

# Read a file
curl http://localhost:8080/ds3/a/b/c.txt

# List directory contents
curl http://localhost:8080/ds3/a/b/

# Delete a file
curl -X DELETE http://localhost:8080/ds3/a/b/c.txt
```

---

## Local File System Implementation
### On-Disk Structures
The local file system uses the following on-disk structures:
- **Super Block**: Metadata about the file system.
- **Bitmaps**: Tracks allocated inodes and data blocks.
- **Inode Table**: Metadata for files and directories.
- **Data Blocks**: Stores file and directory contents.

### Directory Structure
- Each directory has an inode and one or more data blocks.
- Directories include two special entries:
  - `.` (self)
  - `..` (parent)
- Unused directory entries have an inode number of `-1`.

### Crash Consistency
To maintain consistency, all disk writes are carefully ordered. Transactions ensure that:
- Errors do not modify the file system.
- Changes are committed or rolled back atomically.

---

## Error Handling
Errors return appropriate HTTP responses:
- **Resource not found**: `ClientError::notFound()`
- **Insufficient storage**: `ClientError::insufficientStorage()`
- **Conflict**: `ClientError::conflict()` (e.g., creating a directory where a file exists).
- **Bad request**: `ClientError::badRequest()`

---

## Utilities
Three command-line utilities are provided to debug and inspect the file system:

### `ds3ls`
Lists all files and directories in a disk image recursively, starting from the root. Entries are printed in a depth-first order.

### `ds3cat`
Prints the contents of a file to standard output. It includes both the file’s data blocks and actual data.

### `ds3bits`
Displays metadata about the file system, including the super block and inode/data bitmaps.

---

## Development Notes
- API handlers are implemented in `DistributedFileSystemService.cpp`.
- Use provided tools like `mkfs` to create file system images.
- Test APIs with `cURL` or similar HTTP clients.

### Example Disk Utilities
- Use `LocalFileSystem` for read/write operations.
- Implement transaction handling using `beginTransaction`, `commit`, and `rollback`.

---

This project demonstrates the fundamental principles of distributed storage systems, enabling scalable and efficient file management across a network.

