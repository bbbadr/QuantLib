
/*
 Copyright (C) 2002 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mixedscheme.hpp
    \brief Mixed (explicit/implicit) scheme for finite difference methods

    \fullpath
    ql/FiniteDifferences/%mixedscheme.hpp
*/

// $Id$

#ifndef quantlib_mixed_scheme_h
#define quantlib_mixed_scheme_h

#include <ql/FiniteDifferences/finitedifferencemodel.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        //! Mixed (explicit/implicit) scheme for finite difference methods
        /*! See sect. \ref findiff for details on the method.

            In this implementation, the passed operator must be derived
            from either TimeConstantOperator or TimeDependentOperator.
            Also, it must implement at least the following interface:

            \code
            typedef ... arrayType;

            // copy constructor/assignment
            // (these will be provided by the compiler if none is defined)
            Operator(const Operator&);
            Operator& operator=(const Operator&);

            // inspectors
            Size size();

            // modifiers
            void setTime(Time t);

            // operator interface
            arrayType applyTo(const arrayType&);
            arrayType solveFor(const arrayType&);
            static Operator identity(Size size);

            // operator algebra
            Operator operator*(double, const Operator&);
            Operator operator+(const Operator&, const Operator&);
            Operator operator+(const Operator&, const Operator&);
            \endcode

            \warning The differential operator must be linear for
            this evolver to work.

            \todo add Douglas Scheme
        */
        template <class Operator>
        class MixedScheme  {
            friend class FiniteDifferenceModel<MixedScheme<Operator> >;
          protected:
            // typedefs
            typedef typename Operator::arrayType arrayType;
            typedef Operator operatorType;
            // constructors
            MixedScheme(const Operator& L, double theta)
            : L_(L), I_(Operator::identity(L.size())), dt_(0.0), theta_(theta) {}
            void step(arrayType& a, Time t);
            void setStep(Time dt) {
                dt_ = dt;
                if (theta_!=1.0) // there is an explicit part
                    explicitPart_ = I_-((1.0-theta_) * dt_)*L_;
                if (theta_!=0.0) // there is an implicit part
                    implicitPart_ = I_+(     theta_  * dt_)*L_;
            }
            Operator L_;
            Operator I_;
            Operator explicitPart_, implicitPart_;
            Time dt_;
            double theta_;
        };

        // inline definitions

        template <class Operator>
        inline void MixedScheme<Operator>::step(arrayType& a, Time t) {
            if (theta_!=1.0) { // there is an explicit part
                if (L_.isTimeDependent()) {
                    L_.setTime(t);
                    explicitPart_ = I_-((1.0-theta_) * dt_)*L_;
                }
                a = explicitPart_.applyTo(a);
            }
            if (theta_!=0.0) { // there is an implicit part
                if (L_.isTimeDependent()) {
                    L_.setTime(t-dt_);
                    implicitPart_ = I_+(     theta_  * dt_)*L_;
                }
                a = implicitPart_.solveFor(a);
//                a = implicitPart_.SOR(a, 1e-8);
            }
        }

    }

}


#endif
