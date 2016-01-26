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

  (import $print_i32 "spectest" "print" (param i32))

  (func $setter (param i32) (result i32)
    (local i32)
    (block
     (set_local 1 (i32.const 0))
    (label
      (loop
        (if_else
          (i32.eq (get_local 0) (i32.const 0))
          (br 1)
          (block
            ;; Store the n - i in each cell: luckily this is in local 0.
            (i32.store (i32.mul (get_local 1) (i32.const 4)) 
                       (get_local 0))
            (set_local 1 (i32.add (get_local 1) (i32.const 1)))
            (set_local 0 (i32.sub (get_local 0) (i32.const 1)))
          )
        )

        (br 0)
      )
    )
    )
    (return (get_local 0))
  )

  (func $bubble_sort (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    (block

    ;; Local 0 : n, which we will * 4.
    ;; Local 1 : the outer loop IV.
    ;; Local 2 : the inner loop IV.
    ;; Local 3 : inner loop limit: 4 * n - 4 - (local 1).
    ;; Local 4 & 5: temporary storage

    ;; Get it * 4 to make the IV go 4 by 4 and not have the multiply in the address calculation.
    (set_local 0 (i32.mul (get_local 0) (i32.const 4)))
    (set_local 1 (i32.const 0))
    (call_import $print_i32 (i32.load (i32.const 0)))

    (label
      ;; Outer loop.
      (loop
        (if_else
          (i32.eq (get_local 1) (get_local 0))
          (br 1)
          (block
            ;; Set inner loop IV to 0.
            (set_local 2 (i32.const 0))
            ;; Calculate max number: (max * 4) - (4 + outer IV).
            (set_local 3 (i32.sub 
                          (get_local 0) 
                          (i32.add (get_local 1) (i32.const 4))))

            (label
              ;; Inner loop.
              (loop
                 (if_else
                  (i32.eq (get_local 2) (get_local 3))
                  (br 1)
                  (block
                     ;; Load first element.
                     (set_local 4 (i32.load (get_local 2)))
                     (set_local 5 (i32.load (i32.add (get_local 2) (i32.const 4))))

                     ;; Check if 4 > 5?
                     (if
                       (i32.gt (get_local 4) (get_local 5))
                       (block
                        (i32.store (get_local 2) (get_local 5))
                        (i32.store (i32.add (get_local 2) (i32.const 4)) (get_local 4))
                       )
                     )

                     ;; Inner loop IV Increment.
                     (set_local 2 (i32.add (get_local 2) (i32.const 4)))
                  )
                 )
                (br 0)
              )
            )

            ;; Outer loop IV Increment.
            (set_local 1 (i32.add (get_local 1) (i32.const 4)))
          )
        )

        (br 0)
      )
    )
    )
    (return (get_local 1))
  )

  ;; Iterative bubble named
  (export "bubble_sort" $bubble_sort)
  (export "setter" $setter)
)

(assert_return (invoke "setter" (i32.const 25)) (i32.const 0))
(assert_return (invoke "bubble_sort" (i32.const 25)) (i32.const 300))
