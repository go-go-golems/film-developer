## Refactor tasks

- [ ] Figure out how to the models for settings and process are handled
- [ ] Move the main timer for the app to the main class, out of the view
- [ ] Figure out the locking / different threads

## Bugs

- [ ] Fix skipping so that you can skip while paused. Seems to happen after confirmation as well, I had the developer step show 0/0 after a restart.
- [ ] I was able to confuse it so that it wouldn't start and was showing 0/0, I think by ausing / resuming on the last step when it was already finished. Restart should probably clear the flag.
