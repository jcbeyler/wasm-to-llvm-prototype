;; Copyright (c) 2015 Intel Corporation
;;
;; Licensed under the Apache License, Version 2.0 (the "License");
;; you may not use this file except in compliance with the License.
;; You may obtain a copy of the License at
;;
;;      http://www.apache.org/licenses/LICENSE-2.0
;;
;; Unless required by applicable law or agreed to in writing, software
;; distributed under the License is distributed on an "AS IS" BASIS,
;; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;; See the License for the specific language governing permissions and
;; limitations under the License.

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
    (local i32 i32)
    (block
    (set_local 1 (i32.const 0))

    ;; Get it * 4 to make the IV go 4 by 4 and not have the multiply in the address calculation.
    (set_local 0 (i32.mul (get_local 0) (i32.const 4)))

    ;; LLVM is really not nice with its generation if I do a count up.
    ;; Let's do a count down for the iterator in location 2.
    (set_local 2 (i32.sub (get_local 0) (i32.const 4)))

    (label
      (loop
        (if
          (i32.eq (get_local 2) (i32.const 0))
          (break 0)
          (block
            (set_local 1 (i32.add (get_local 1) (i32.load (get_local 2))))
            (set_local 2 (i32.sub (get_local 2) (i32.const 4)))
          )
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
