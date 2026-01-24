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
    - [x] cache each turn and step through boards
    - [x] pawn moves
    - [x] knight moves
    - [~] bishop moves
    - [ ] rook moves
    - [ ] Cleanup A - assert macro?
- [ ] On-screen text box to paste pgn data

~~~~~~~~

### CLEANUP
A: assert/validate return values from functions finding indexes into the array throughout:
    - get_index_from_move, char_to_file_or_rank
B: manually create a PGN which tests edge-cases?:
    - en-passant, different levels of disambiguation