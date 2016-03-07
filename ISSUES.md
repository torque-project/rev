= Known Issues =

This is a list of currently known issue with the software

== Assigning local variables ==

There is a bug that causes the software to crash when local variables are assigned from function 
parameters of the same name like so:

```clojure
(fn [x]
  (let [x x]
    x))
```

