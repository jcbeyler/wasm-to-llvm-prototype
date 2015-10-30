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
  (memory 40000000 4)

  (func $setter (param i32) (result i32)
   (local i32 i32)

   (block
    ;; Get two pointers for each vector. 
    (set_local 1 (i32.const 0))
    (set_local 2 (get_local 0))

    (label
     (loop
      (if
       (i32.eq (get_local 0) (i32.const 0))
       (break 0)
       (block
        (i32.store (i32.mul (get_local 1) (i32.const 4)) (get_local 1))
        (i32.store (i32.mul (get_local 2) (i32.const 4)) (get_local 1))
        (set_local 1 (i32.add (get_local 1) (i32.const 1)))
        (set_local 2 (i32.add (get_local 2) (i32.const 1)))
        (set_local 0 (i32.sub (get_local 0) (i32.const 1)))
       )
      )
     )
    )
   )
   (return (get_local 0))
  )

  (func $vector_mul (param i32) (result i32)
    (local i32 i32 i32)
    (block
      (set_local 3 (i32.const 0))

      ;; Local 0 is the number of elements
      ;; Local 1 is the pointer to the first vector
      ;; Local 2 is the pointer to the second vector
      ;; Local 3 is the vector_mul

      ;; Get it * 4 to make the IV go 4 by 4 and not have the multiply in the address calculation.
      (set_local 0 (i32.mul (get_local 0) (i32.const 4)))

      ;; LLVM is really not nice with its generation if I do a count up.
      ;; Let's do a count down for the iterator in location 2.
      (set_local 1 (i32.sub (get_local 0) (i32.const 4)))
      (set_local 2 (i32.add (get_local 0) (get_local 1)))

      (label
        (loop
          (if
            (i32.eq (get_local 1) (i32.const 0))
            (break 0)
            (block
              (set_local 3 (i32.add (get_local 3) (i32.mul (i32.load (get_local 1)) (i32.load (get_local 2)))))
              (set_local 1 (i32.sub (get_local 1) (i32.const 4)))
              (set_local 2 (i32.sub (get_local 2) (i32.const 4)))
            )
          )
        )
      )
    )
    (return (get_local 3))
  )

  (export "vector_mul" $vector_mul)
  (export "setter" $setter)
)

(assert_return (invoke "setter" (i32.const 1000)) (i32.const 0))
(assert_return (invoke "vector_mul" (i32.const 1000)) (i32.const 332833500))

