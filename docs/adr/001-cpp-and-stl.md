# ADR-001: Plain C++17 with the standard library only

- **Status:** Accepted
- **Date:** 2026-11-18

## Context

A small warehouse tool must run on any office computer, start instantly and be easy to build. The goal of the project is also to show solid C++ fundamentals: classes, containers, exceptions, tests.

## Options considered

| Option | Pros | Cons |
|---|---|---|
| C++17 + STL only | No dependencies, builds everywhere, shows the fundamentals | Some code (CSV, tables) written by hand |
| C++ + Qt | Graphical interface | Big dependency, hides the core logic behind the framework |
| C++ + SQLite | Real database, queries | Extra library; CSV is enough for one warehouse |

## Decision

C++17 and the standard library. GoogleTest is the only dependency, and only for tests.

## Consequences

- Builds with GCC, Clang and MSVC with one CMake command.
- Data is stored in CSV files that can be opened in Excel.
- A real multi-user system would need a database (see ADR-002).
