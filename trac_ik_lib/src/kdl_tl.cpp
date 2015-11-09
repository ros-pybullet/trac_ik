/********************************************************************************
Copyright (c) 2015, TRACLabs, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, 
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software 
       without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************/

#include <trac_ik/kdl_tl.hpp>
#include <boost/date_time.hpp>


namespace KDL
{
  ChainIkSolverPos_TL::ChainIkSolverPos_TL(const Chain& _chain, const JntArray& _q_min, const JntArray& _q_max, double _maxtime, double _eps, bool _random_restart):
    chain(_chain), q_min(_q_min), q_max(_q_max), vik_solver(_chain), fksolver(_chain), delta_q(_chain.getNrOfJoints()),
    maxtime(_maxtime),eps(_eps),rr(_random_restart)
  {

  }

  int ChainIkSolverPos_TL::CartToJnt(const KDL::JntArray &q_init, const KDL::Frame &p_in, KDL::JntArray &q_out, const KDL::Twist _bounds) {

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration timediff;
    q_out = q_init;
    bounds = _bounds;

    aborted=false;
           
    double time_left;


    do {
      fksolver.JntToCart(q_out,f);
      delta_twist = diff(f,p_in);
        
      if (std::abs(delta_twist.vel.x()) <= std::abs(bounds.vel.x()))
        delta_twist.vel.x(0);
      
      if (std::abs(delta_twist.vel.y()) <= std::abs(bounds.vel.y()))
        delta_twist.vel.y(0);
      
      if (std::abs(delta_twist.vel.z()) <= std::abs(bounds.vel.z()))
        delta_twist.vel.z(0);
      
      if (std::abs(delta_twist.rot.x()) <= std::abs(bounds.rot.x()))
        delta_twist.rot.x(0);
      
      if (std::abs(delta_twist.rot.y()) <= std::abs(bounds.rot.y()))
        delta_twist.rot.y(0);
      
      if (std::abs(delta_twist.rot.z()) <= std::abs(bounds.rot.z()))
        delta_twist.rot.z(0);


      if(Equal(delta_twist,Twist::Zero(),eps))
        return 0;
        
      
      vik_solver.CartToJnt(q_out,delta_twist,delta_q);
      KDL::JntArray q_curr;
      
      Add(q_out,delta_q,q_curr);
      
      for(unsigned int j=0; j<q_min.rows(); j++) {
        if(q_curr(j) < q_min(j))
          q_curr(j) = q_min(j);
      }
      
      for(unsigned int j=0; j<q_max.rows(); j++) {
        if(q_curr(j) > q_max(j))
          q_curr(j) = q_max(j);
      }
      
      Subtract(q_out,q_curr,q_out);
      
      if (q_out.data.isZero(1e-8)) {
        if (rr) {
          for (unsigned int j=0; j<q_out.rows(); j++) 
            q_curr(j)=fRand(q_min(j),q_max(j));
        }
        else {
          q_out=q_curr;
          return -3;
        }
      }

      q_out=q_curr;
     
      timediff=boost::posix_time::microsec_clock::local_time()-start_time;
      time_left = maxtime - timediff.total_milliseconds()/1000.0;
    } while (time_left > 0 && !aborted);
    
    return -3;
  }

  ChainIkSolverPos_TL::~ChainIkSolverPos_TL()
  {
  }

  void ChainIkSolverPos_TL::abort() {
    aborted = true;
  }


}
