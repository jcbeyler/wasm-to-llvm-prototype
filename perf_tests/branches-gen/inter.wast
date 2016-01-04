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

          (if_else
            (i32.eq (i32.rem_u (get_local 2) (i32.const 2)) (i32.const 1))
            (set_local 2 (i32.add
                            (i32.mul (get_local 2) (i32.const 3))
                            (i32.const 1)))
            (set_local 2 (i32.div
                            (get_local 2)
                            (i32.const 2)))
          )

          (if_else
            (i32.eq (i32.rem_u (get_local 3) (i32.const 2))  (i32.const 1))
            (set_local 3 (i32.add
                            (i32.mul (get_local 3) (i32.const 3))
                            (i32.const 1)))
            (set_local 3 (i32.div
                            (get_local 3)
                            (i32.const 2)))
          )

          (if_else
            (i32.eq (i32.rem_u (get_local 4) (i32.const 2))  (i32.const 1))
            (set_local 4 (i32.add
                            (i32.mul (get_local 4) (i32.const 3))
                            (i32.const 1)))
            (set_local 4 (i32.div
                            (get_local 4)
                            (i32.const 2)))
          )

          (if_else
            (i32.eq (i32.rem_u (get_local 5) (i32.const 2))  (i32.const 1))
            (set_local 5 (i32.add
                            (i32.mul (get_local 5) (i32.const 3))
                            (i32.const 1)))
            (set_local 5 (i32.div
                            (get_local 5)
                            (i32.const 2)))
          )

          (if_else
            (i32.eq (i32.rem_u (get_local 6) (i32.const 2))  (i32.const 1))
            (set_local 6 (i32.add
                            (i32.mul (get_local 6) (i32.const 3))
                            (i32.const 1)))
            (set_local 6 (i32.div
                            (get_local 6)
                            (i32.const 2)))
          )

          (if_else
            (i32.eq (i32.rem_u (get_local 7) (i32.const 2))  (i32.const 1))
            (set_local 7 (i32.add
                            (i32.mul (get_local 7) (i32.const 3))
                            (i32.const 1)))
            (set_local 7 (i32.div
                            (get_local 7)
                            (i32.const 2)))
          )

          (if_else
            (i32.eq (i32.rem_u (get_local 8) (i32.const 2))  (i32.const 1))
            (set_local 8 (i32.add
                            (i32.mul (get_local 8) (i32.const 3))
                            (i32.const 1)))
            (set_local 8 (i32.div
                            (get_local 8)
                            (i32.const 2)))
          )

          (if_else
            (i32.eq (i32.rem_u (get_local 9) (i32.const 2))  (i32.const 1))
            (set_local 9 (i32.add
                            (i32.mul (get_local 9) (i32.const 3))
                            (i32.const 1)))
            (set_local 9 (i32.div
                            (get_local 9)
                            (i32.const 2)))
          )

          (if_else
            (i32.eq (i32.rem_u (get_local 10) (i32.const 2))  (i32.const 1))
            (set_local 10 (i32.add
                            (i32.mul (get_local 10) (i32.const 3))
                            (i32.const 1)))
            (set_local 10 (i32.div
                            (get_local 10)
                            (i32.const 2)))
          )
