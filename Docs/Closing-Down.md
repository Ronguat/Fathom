# Closing down a session

**Trigger: one of four conditions, or the designer saying so.** Scope done; a stop-list item
reached; the session brief's budget reached; a dead end after the fallback. Run the whole
procedure every time; it is what makes an unattended session reviewable.

1. **Make the editor state safe.** Save what you touched by name, then read `git status`; seeing
   the files listed is the check, calling save is not. Decline any restore prompt on reopening,
   and reset `Saved/Autosaves/PackageRestoreData.json` before an unattended relaunch.
2. **Run the loop, and read what it says rather than its exit code.** Once the Harness rung ships:
   the rows the session's changes can reach, selected by mechanism, at every latency they carry;
   a change to anything shared runs everything. A red row blocks the push. An unproven mutation is
   worse than a red. Until the loop exists, say so in the handoff.
3. **Audit the always-read file, then run both checks.** Re-read `CLAUDE.md` in full against two
   questions: is this a rule, or the story of how it was learned; and does it need re-reading every
   session by someone who may not touch the system it binds. Then `Tools/CommentCheck/comment-check.sh`
   and `Tools/DocsCheck/docs-check.sh`: clear every FAIL, read every WARN. A volume warning names a
   file whose comments grew; judge it, and raise its line in `Tools/CommentCheck/baseline.txt` only
   with the reason in the commit, never with `--baseline` to make a run green. The ownership check
   lists agent commits on the reference model; those rows are for the designer.
4. **Discharge what you fixed.** A trap discharged in the same commit, saying what discharged it; a
   supersession row for anything superseded; a date and a quoted search on every absence claim. If
   a symbol or a value changed, grep it across `Docs/` and `CLAUDE.md`, reconcile every hit, and
   quote the greps in the handoff.
5. **Fill the review queue.** Every WHAT decided alone this session, one row each, newest first.
6. **Commit, then push if the gate is green.** Verified units to main, the trailer on every commit.
   The gate: the binary newer than every source, the reachable rows green, both checks passing, the
   handoff written. Then `git push origin main` and nothing else. Unverified work goes to
   `wip/<sub-slice>`, named in the handoff; main never carries it.
7. **Update the focus, and route a shipped rung.** The ladder line in `CLAUDE.md` is the only place
   the order lives. A shipped rung keeps its strikethrough there and routes everything else by the
   table under it; verify each consequence landed before removing its old home, in that order.
8. **Write the handoff as the next session's brief**, at the top of the newest dated entry: where to
   pick up, the scope proposed, the budget, what is verified versus merely written, what is open,
   and what was done beyond the plan, or that nothing was. **Name anything claimed but not
   verified**; it is the item most likely to be believed next session and least likely to be
   re-checked.
9. **Check memory points rather than restates.** Anything a future instance needs is in the repo.
10. **Title the session, five words maximum.** What it did, not what it touched.
11. **Close the editor, last.** Announce it, quit gracefully through the Python runner, confirm the
    process is gone and the restore file reads `Packages: []`. Every session then starts the same
    way: editor closed, tree clean, restore file empty. It also frees the machine and the port the
    other project's editor needs. The one exception: the designer, present, says to leave it open.
