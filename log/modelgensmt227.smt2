(set-logic QF_BV)
(declare-fun v1 () (_ BitVec 1))
(declare-fun v2 () (_ BitVec 9))
(declare-fun v3 () (_ BitVec 4))
(define-fun $e4 () (_ BitVec 5) (_ bv30 5))
(define-fun $e5 () (_ BitVec 4) (_ bv14 4))
(define-fun $e6 () (_ BitVec 9) (_ bv0 9))
(define-fun $e7 () (_ BitVec 12) (_ bv0 12))
(define-fun $e8 () (_ BitVec 13) (_ bv0 13))
(define-fun $e9 () (_ BitVec 3) (_ bv0 3))
(define-fun $e10 () (_ BitVec 11) (_ bv0 11))
(define-fun $e11 () (_ BitVec 13) (_ bv8190 13))
(define-fun $e12 () (_ BitVec 5) (_ bv2 5))
(define-fun $e13 () (_ BitVec 4) (_ bv0 4))
(define-fun $e14 () (_ BitVec 1) (ite (= $e6 v2) #b1 #b0))
(define-fun $e15 () (_ BitVec 12) (concat $e10 v1))
(define-fun $e16 () (_ BitVec 5) (concat $e13 v1))
(define-fun $e17 () (_ BitVec 5) (bvadd (bvnot $e4) (bvnot $e16)))
(define-fun $e18 () (_ BitVec 5) (bvadd (bvnot $e4) (bvnot $e17)))
(define-fun $e19 () (_ BitVec 1) ((_ extract 4 4) $e18))
(define-fun $e20 () (_ BitVec 1) ((_ extract 4 4) $e17))
(define-fun $e21 () (_ BitVec 1) (bvand $e19 (bvnot $e20)))
(define-fun $e22 () (_ BitVec 4) ((_ extract 3 0) $e18))
(define-fun $e23 () (_ BitVec 4) ((_ extract 3 0) $e17))
(define-fun $e24 () (_ BitVec 1) (ite (bvult $e22 $e23) #b1 #b0))
(define-fun $e25 () (_ BitVec 1) (bvand (bvnot $e19) (bvnot $e20)))
(define-fun $e26 () (_ BitVec 1) (bvand $e24 $e25))
(define-fun $e27 () (_ BitVec 1) (bvand $e19 $e20))
(define-fun $e28 () (_ BitVec 1) (bvand $e24 $e27))
(define-fun $e29 () (_ BitVec 1) (bvand (bvnot $e26) (bvnot $e28)))
(define-fun $e30 () (_ BitVec 1) (bvand (bvnot $e21) $e29))
(define-fun $e31 () (_ BitVec 16) (concat $e15 $e13))
(define-fun $e32 () (_ BitVec 16) (bvlshr $e31  ((_ zero_extend 12) v3)))
(define-fun $e33 () (_ BitVec 13) ((_ extract 12 0) $e32))
(define-fun $e34 () (_ BitVec 13) (ite (= #b1 $e14) $e33 $e8))
(define-fun $e35 () (_ BitVec 13) (bvand $e11 (bvnot $e34)))
(define-fun $e36 () (_ BitVec 13) (bvand (bvnot $e11) $e34))
(define-fun $e37 () (_ BitVec 13) (bvand (bvnot $e35) (bvnot $e36)))
(define-fun $e38 () (_ BitVec 1) ((_ extract 12 12) $e37))
(define-fun $e39 () (_ BitVec 12) ((_ extract 11 0) $e37))
(define-fun $e40 () (_ BitVec 1) (ite (= $e7 (bvnot $e39)) #b1 #b0))
(define-fun $e41 () (_ BitVec 1) (bvand $e38 (bvnot $e40)))
(define-fun $e42 () (_ BitVec 1) (bvand (bvnot $e30) (bvnot $e41)))
(define-fun $e43 () (_ BitVec 4) (concat $e9 v1))
(define-fun $e44 () (_ BitVec 1) (ite (bvult (bvnot $e5) $e43) #b1 #b0))
(define-fun $e45 () (_ BitVec 1) (bvand (bvnot v1) $e44))
(define-fun $e46 () (_ BitVec 5) (bvadd $e12 (bvnot $e16)))
(define-fun $e47 () (_ BitVec 1) ((_ extract 4 4) $e46))
(define-fun $e48 () (_ BitVec 1) (bvand (bvnot $e45) $e47))
(define-fun $e49 () (_ BitVec 4) ((_ extract 3 0) $e46))
(define-fun $e50 () (_ BitVec 3) (ite (= #b1 $e45) (bvnot $e9) $e9))
(define-fun $e51 () (_ BitVec 4) (concat $e50 (bvnot v1)))
(define-fun $e52 () (_ BitVec 4) (concat $e50 $e44))
(define-fun $e53 () (_ BitVec 4) (bvand $e51 $e52))
(define-fun $e54 () (_ BitVec 1) (ite (bvult $e49 $e53) #b1 #b0))
(define-fun $e55 () (_ BitVec 1) (bvand (bvnot $e45) (bvnot $e47)))
(define-fun $e56 () (_ BitVec 1) (bvand $e54 $e55))
(define-fun $e57 () (_ BitVec 1) (bvand $e45 $e47))
(define-fun $e58 () (_ BitVec 1) (bvand $e54 $e57))
(define-fun $e59 () (_ BitVec 1) (bvand (bvnot $e48) (bvnot $e56)))
(define-fun $e60 () (_ BitVec 1) (ite (= v1 (bvnot $e59)) #b1 #b0))
(assert (not (= (bvnot $e42) #b0)))
(assert (not (= $e60 #b0)))
(assert (not (= (bvnot $e58) #b0)))
(check-sat)
(exit)
