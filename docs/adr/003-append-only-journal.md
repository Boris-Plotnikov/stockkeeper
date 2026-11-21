# ADR-003: Append-only journal of movements

- **Status:** Accepted
- **Date:** 2026-11-18

## Context

In real warehouses the main question after a shortage is "who took it and when?". If quantities can simply be overwritten, the answer is lost.

## Decision

Every change of quantity writes a `Movement` with the balance after the operation. Movements are never edited or deleted. Stocktaking (`count`) also writes a movement, so the difference between the counted and the expected quantity is visible.

## Consequences

- Full history for every item, the current quantity can be checked against the journal.
- The journal grows over time; archiving old years is a possible next step.
