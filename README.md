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
    - [x] Board
    - [x] Pieces
- [x] Render default game state
    - [x] Grid
    - [x] Piece sprites
- [x] Parse PGN movetext
    - [x] Strip tag pairs
    - [x] Read turn
    - [x] Populate turn buffer
        - [x] Castle
        - [x] Promotion
- [!] Step through move history on board
    - [x] figure out turn history buffer/pgn game turns relationship
    - [x] cache each turn and step through boards
    - [x] pawn moves
    - [x] knight moves
    - [x] bishop moves
    - [x] rook moves
    - [x] queen moves
    - [x] castle
    - [x] king moves
    - [x] promotion
    - [x] fix pgn_reader reading all turns
    - [x] "trace_clear_line" - add way to specify which type of line
    - [x] Fix pawn move/promotion
    - [x] Add rank disambiguation if not a file
    - [~] Check for king pins when moving piece
    - [ ] Helpers for similar functions across all pieces
    - [ ] Cleanup A - assert macro?
- [ ] Mass PGN tester
    - [ ] read lots of PGNs in a go, stop on errors?
- [!] Strip variations from PGN_Reader
    - [ ] test this more
- [!] Flip board
    - [ ] Make this more generic
- [ ] Game effects
- [ ] On-screen text box to paste pgn data
- [ ] Better PGN error handling
- [ ] Return error codes and stop input

~~~~~~~~

### CLEANUP
A: assert/validate return values from functions finding indexes into the array throughout:
    - get_index_from_move, char_to_file_or_rank
C: Can we simplify the hunting moves? (all pieces)
D: manually create a PGN which tests edge-cases?:
    - en-passant, different levels of disambiguation
    - queenside castle
    - capturepromotion
    - bishops/queens on same diagonal
    - trace_clear_line() with diagonal rook or straight bishop with a clear line
