1. **Read `handleTouch` and `drawButtons` logic**
   - I will use `sed` to read the exact lines of `handleTouch` and `drawButtons` from `src/main.cpp` to understand how buttons are defined and handled.
2. **Add `drawSingleButton` helper and update `drawButtons`**
   - I will use `replace_with_git_merge_diff` to add a `drawSingleButton(int x, int y, int w, int h, const char* label, uint16_t color, bool pressed)` helper function in `src/main.cpp` to centralize button rendering.
   - I will update `drawButtons` to use this new helper.
3. **Add visual feedback to `handleTouch`**
   - I will use `replace_with_git_merge_diff` in `src/main.cpp` inside `handleTouch` to redraw the touched button with `pressed=true`, wait `50ms`, and redraw it with `pressed=false` before executing its action.
4. **Run PlatformIO compile/test**
   - Run `platformio run -e cyd` to make sure the code compiles successfully without regressions.
5. **Complete pre-commit steps to ensure proper testing, verification, review, and reflection are done.**
   - Run the pre commit instructions and check everything off.
6. **Submit PR**
   - Submit the change with the PR title `🎨 Palette: [UX improvement]` and describe what, why, before/after, and accessibility.
