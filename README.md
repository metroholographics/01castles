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
- [x] Mass PGN tester
- [!] On-screen text box to paste pgn data
    - [x] Render input texture
    - [!] Paste clipboard to box
        - [x] Register paste and create new game
        - [!] Display game text on screen
            - [x] Paste text 
            - [x] Add scroll
            - [x] Make game not crash on bad paste
            - [ ] Tidy
- [!] Strip variations from PGN_Reader
    - [ ] test this more
- [!] Flip board
    - [ ] Make this more generic
- [ ] Game effects
- [ ] Better PGN error handling

~~~~~~~~

### CLEANUP
A: assert/validate return values from functions finding indexes into the array throughout:
    - get_index_from_move, char_to_file_or_rank
