# ADR-002: CSV files instead of a database

- **Status:** Accepted
- **Date:** 2026-11-18

## Context

One person runs the warehouse at a time. The data must survive a restart and be readable without the program.

## Decision

Two CSV files: `items.csv` (current state) and `movements.csv` (journal). The note is always the last column, so it may contain commas. A broken line is reported with the file name and line number.

## Consequences

- Easy to back up, compare and open in Excel.
- No parallel access: two users at the same time would overwrite each other. The next step for multiple users would be SQLite or a server with PostgreSQL.
