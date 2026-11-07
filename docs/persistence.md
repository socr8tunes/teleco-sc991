# Saved state (retained memory) and firmware updates

The calculator keeps Ans, variables A-F/X/Y/M, matrices and vectors, statistics data, replay history, display settings and
contrast across power-off. State is stored as one blob: magic "TSC9", a **format version**, structure sizes, the data and a CRC32
(`core/ui/persist.c`). The previous blob stays valid until a new one is fully committed, so removing the battery during a save
cannot destroy the last good state.

## Incompatibility rule
- The stored blob is a raw image of the calculator state. **Whenever the layout of the state structure changes (new fields,
  new settings) the format version is increased, and a blob with any other version is discarded, never converted.**
  Token codes no longer force a version change: since version 7 the replay history is stored with **frozen wire IDs**
  (`core/parser/token_ids.h`, list in `tests/exam/frozen_token_ids.txt`), independent of the compile-time enum.
  After such a firmware update the calculator starts clean: memory, variables, history, matrices/vectors, statistics data and
  settings return to their defaults. Nothing is misread and nothing crashes.
- Version history: 3 -> 4 when GCD/LCM/Quot/Rem/PFact/BinomPD/BinomCD/PoisPD/PoisCD/InvN were added (token codes shifted; ctx grew).
  Any pilot unit with firmware older than this loses its stored state once on update.
- Version 6 -> 7: the history changed from enum values to frozen wire IDs. Pilot units with older firmware lose their stored
  state once on update.
- **Freeze rules (before pilot hardware distribution):** a released wire ID is never reused or changed; new tokens take the
  next free ID in their group (groups leave reserved gaps); the constants, conversions, statistics-variable and variable
  tables only grow (`tests/exam/frozen_aux.txt`); `tests/unit/test_token_ids.c` fails on any violation. An unknown ID in an
  otherwise valid blob (a newer firmware's history) drops only the history and keeps settings and memory.
- Layout changes to the state structure still discard the blob. Policy for pilot units: such changes are batched into as few
  firmware releases as possible and announced in the release notes; a migration path is added only if the product owner asks.
- The same applies to a corrupt blob (CRC mismatch) and to a blob of a different size.
- Pilot instructions: tell testers to write down anything they want to keep before installing a new build, and re-enter
  settings afterwards. The web simulator never persists.
- Tests: `tests/unit/test_shell.c` (`test_old_version_discarded`, `test_corruption_and_failed_save`), `tests/unit/test_token_ids.c` (freeze and history round trip).
