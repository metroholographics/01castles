# 01castles

- Editable chess board with PGN movetext parser to initialise board
- Playable game (no ai) with move history

---

### TODO

~~~~~~~~

v0.1
- [x] Initialise SDL environment
- [x] Window, Renderer
- [x] Loop, FPS
- [x] Game data
- [x] Render default game state
- [x] Parse PGN movetext
- [x] Step through move history on board
- [!] Mass PGN tester
    - [x] test environment
    - [x] castles error handling
    - [x] stop on error in test
    - [!] run more tests
        - [x] fix pin check - messing with the piece index doesn't work
        - [x] pgn_reader updating 'c'
        - [~] more info for testing - game length (manual?)
        - [ ] run more tests
- [!] Strip variations from PGN_Reader
    - [ ] test this more
- [!] Flip board
    - [ ] Make this more generic
- [ ] Cleanup A - assert macro?
- [ ] Game effects
- [ ] On-screen text box to paste pgn data
- [ ] Better PGN error handling
- [ ] Return error codes and stop input

~~~~~~~~

### CLEANUP
A: assert/validate return values from functions finding indexes into the array throughout:
    - get_index_from_move, char_to_file_or_rank
D: manually create a PGN which tests edge-cases?:
    - en-passant, different levels of disambiguation
    - queenside castle
    - capturepromotion
    - bishops/queens on same diagonal
    - trace_clear_line() with diagonal rook or straight bishop with a clear line
