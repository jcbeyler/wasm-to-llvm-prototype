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

       )
      )
      (br 0)
    )
   )


   (return (i32.add
            (i32.add
             (i32.add
              (i32.add
               (i32.add
                (i32.add
                 (i32.add
                  (i32.add
                    (get_local 2)
                    (get_local 3))
                    (get_local 4))
                    (get_local 5))
                    (get_local 6))
                    (get_local 7))
                    (get_local 8))
                    (get_local 9))
                    (get_local 10))
   )
  )

  (func $branches (param i32) (result i32)
    (call $branches_intern (get_local 0)
      (i32.const 1)
      (i32.const 1)
      (i32.const 2)
      (i32.const 3)
      (i32.const 4)
      (i32.const 5)
      (i32.const 6)
      (i32.const 7)
      (i32.const 8)
      (i32.const 9)
     )
  )

  ;; Iterative sum named
  (export "branches" $branches)
)

(assert_return (invoke "branches" (i32.const 25)) (i32.const 300))
