# Saved state (retained memory) and firmware updates

The calculator keeps Ans, variables A-F/X/Y/M, matrices and vectors, statistics data, replay history, display settings and
contrast across power-off. State is stored as one blob: magic "TSC9", a **format version**, structure sizes, the data and a CRC32
(`core/ui/persist.c`). The previous blob stays valid until a new one is fully committed, so removing the battery during a save
cannot destroy the last good state.

## Incompatibility rule
- The stored blob is a raw image of the calculator state. **Whenever the layout changes (new fields, new token codes in
  saved history) the format version is increased, and a blob with any other version is discarded, never converted.**
  After such a firmware update the calculator starts clean: memory, variables, history, matrices/vectors, statistics data and
  settings return to their defaults. Nothing is misread and nothing crashes.
- Version history: 3 -> 4 when GCD/LCM/Quot/Rem/PFact/BinomPD/BinomCD/PoisPD/PoisCD/InvN were added (token codes shifted; ctx grew).
  Any pilot unit with firmware older than this loses its stored state once on update.
- The same applies to a corrupt blob (CRC mismatch) and to a blob of a different size.
- Pilot instructions: tell testers to write down anything they want to keep before installing a new build, and re-enter
  settings afterwards. The web simulator never persists.
- Tests: `tests/unit/test_shell.c` (`test_old_version_discarded`, `test_corruption_and_failed_save`).
