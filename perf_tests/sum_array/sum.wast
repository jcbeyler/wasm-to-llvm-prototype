;; (c) 2015 Andreas Rossberg

(module
  (memory 40000 4)

  (func $getter (param i32) (result i32)
    (i32.load (i32.mul (i32.const 4) (get_local 0)))
  )

  (func $setone (param i64) (param i64) (result i32)
    (i64.store (get_local 0) (i64.mul (i64.const 4) (get_local 1)))
  )

  (export "getter"$getter)

  (func $setter (param i32) (result i32)
    (local i32)
    (block
     (set_local 1 (i32.const 0))
    (label
      (loop
        (if
          (i32.eq (get_local 0) (i32.const 0))
          (break 0)
          (block
            (i32.store (i32.mul (get_local 1) (i32.const 4)) (get_local 1))
            (set_local 1 (i32.add (get_local 1) (i32.const 1)))
            (set_local 0 (i32.sub (get_local 0) (i32.const 1)))
          )
        )
      )
    )
    )
    (return (get_local 0))
  )

  (func $sum (param i32) (result i32)
    (local i32)
    (set_local 1 (i32.const 0))

    (label
      (loop
        (if
          (i32.eq (get_local 0) (i32.const 0))
          (break 0)
          (block
            (set_local 1 (i32.add (get_local 1) (i32.load (i32.mul (i32.const 4) (get_local 0)))))
            (set_local 0 (i32.sub (get_local 0) (i32.const 1)))
          )
        )
      )
    )
    (return (get_local 1))
  )

  ;; Iterative sum named
  (export "sum" $sum)
  (export "setter" $setter)
  (export "setone" $setone)
)

(assert_return (invoke "setter" (i32.const 25)) (i32.const 0))
(assert_return (invoke "sum" (i32.const 25)) (i32.const 300))
