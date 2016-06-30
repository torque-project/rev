# Known Issues

This is a list of currently known issue with the software

## Shadowing locals in loop

When letting a symbol in a loop with the same name as a loop binding,
recur will use the wrong stack position during recur. This makes
loops return the wrong result, or loop forever
