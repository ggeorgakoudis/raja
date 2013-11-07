/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file containing RAJA index set iteration template 
 *          methods for OpenMP execution policies.
 *
 *          These methods should work on any platform.
 *
 * \author  Rich Hornung, Center for Applied Scientific Computing, LLNL
 * \author  Jeff Keasler, Applications, Simulations And Quality, LLNL
 *
 ******************************************************************************
 */

#ifndef RAJA_forall_omp_any_HXX
#define RAJA_forall_omp_any_HXX

#include "config.hxx"

#include "datatypes.hxx"

#include "execpolicy.hxx"

#include "ISet.hxx"

#include <omp.h>


namespace RAJA {


//
//////////////////////////////////////////////////////////////////////
//
// Function templates that iterate over range index sets.
//
//////////////////////////////////////////////////////////////////////
//

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for iteration over index range.
 *
 ******************************************************************************
 */
template <typename LOOP_BODY>
RAJA_INLINE
void forall(omp_parallel_for_exec,
            Index_type begin, Index_type end, 
            LOOP_BODY loop_body)
{
#pragma omp parallel for
   for ( Index_type ii = begin ; ii < end ; ++ii ) {
      loop_body( ii );
   }
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for iteration over index range set object.
 *
 ******************************************************************************
 */
template <typename LOOP_BODY>
RAJA_INLINE
void forall(omp_parallel_for_exec,
            const RangeISet& is,
            LOOP_BODY loop_body)
{
   const Index_type begin = is.getBegin();
   const Index_type end   = is.getEnd();
#pragma omp parallel for
   for ( Index_type ii = begin ; ii < end ; ++ii ) {
      loop_body( ii );
   }
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for minloc reduction over index range.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_minloc(omp_parallel_for_exec,
                   Index_type begin, Index_type end,
                   T* min, Index_type *loc,
                   LOOP_BODY loop_body)
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  min_tmp[nthreads];
   Index_type loc_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
       min_tmp[i] = *min ;
       loc_tmp[i] = *loc ;
   }

#pragma omp parallel for
   for ( Index_type ii = begin ; ii < end ; ++ii ) {
      loop_body( ii, &min_tmp[omp_get_thread_num()],
                     &loc_tmp[omp_get_thread_num()] );
   }

   for ( int i = 1; i < nthreads; ++i ) {
      if ( min_tmp[i] < min_tmp[0] ) {
         min_tmp[0] = min_tmp[i];
         loc_tmp[0] = loc_tmp[i];
      }
   }

   *min = min_tmp[0] ;
   *loc = loc_tmp[0] ;
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for minloc reduction over range index set object.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_minloc(omp_parallel_for_exec,
                   const RangeISet& is,
                   T* min, Index_type *loc,
                   LOOP_BODY loop_body)
{
   forall_minloc(omp_parallel_for_exec(),
                 is.getBegin(), is.getEnd(),
                 min, loc,
                 loop_body);
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for maxloc reduction over index range.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_maxloc(omp_parallel_for_exec,
                   Index_type begin, Index_type end,
                   T* max, Index_type *loc,
                   LOOP_BODY loop_body)
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  max_tmp[nthreads];
   Index_type loc_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
       max_tmp[i] = *max ;
       loc_tmp[i] = *loc ;
   }

#pragma omp parallel for 
   for ( Index_type ii = begin ; ii < end ; ++ii ) {
      loop_body( ii, &max_tmp[omp_get_thread_num()],
                     &loc_tmp[omp_get_thread_num()] );
   }

   for ( int i = 1; i < nthreads; ++i ) {
      if ( max_tmp[i] > max_tmp[0] ) {
         max_tmp[0] = max_tmp[i];
         loc_tmp[0] = loc_tmp[i];
      }
   }

   *max = max_tmp[0] ;
   *loc = loc_tmp[0] ;
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for maxloc reduction over range index set object.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_maxloc(omp_parallel_for_exec,
                   const RangeISet& is,
                   T* max, Index_type *loc,
                   LOOP_BODY loop_body)
{
   forall_maxloc(omp_parallel_for_exec(),
                 is.getBegin(), is.getEnd(),
                 max, loc,
                 loop_body);
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for sum reduction over index range.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_sum(omp_parallel_for_exec,
                Index_type begin, Index_type end,
                T* sum,
                LOOP_BODY loop_body)
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  sum_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
      sum_tmp[i] = 0 ;
   }

#pragma omp parallel for
   for ( Index_type ii = begin ; ii < end ; ++ii ) {
      loop_body( ii, &sum_tmp[omp_get_thread_num()] );
   }

   for ( int i = 0; i < nthreads; ++i ) {
      *sum += sum_tmp[i];
   }
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for sum reduction over range index set object.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_sum(omp_parallel_for_exec,
                const RangeISet& is,
                T* sum, 
                LOOP_BODY loop_body)
{
   forall_sum(omp_parallel_for_exec(),
              is.getBegin(), is.getEnd(),
              sum,
              loop_body);
}

//
//////////////////////////////////////////////////////////////////////
//
// Function templates that iterate over range index sets with stride.
//
//////////////////////////////////////////////////////////////////////
//

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for iteration over index range with stride.
 *
 ******************************************************************************
 */
template <typename LOOP_BODY>
RAJA_INLINE
void forall(omp_parallel_for_exec,
            Index_type begin, Index_type end, Index_type stride,
            LOOP_BODY loop_body)
{
#pragma omp parallel for
   for ( Index_type ii = begin ; ii < end ; ii += stride ) {
      loop_body( ii );
   }
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for iteration over range index set with stride object.
 *
 ******************************************************************************
 */
template <typename LOOP_BODY>
RAJA_INLINE
void forall(omp_parallel_for_exec,
            const RangeStrideISet& is,
            LOOP_BODY loop_body)
{
   const Index_type begin  = is.getBegin();
   const Index_type end    = is.getEnd();
   const Index_type stride = is.getStride();
#pragma omp parallel for
   for ( Index_type ii = begin ; ii < end ; ii += stride ) {
      loop_body( ii );
   }
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for minloc reduction over index range with stride.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_minloc(omp_parallel_for_exec,
                   Index_type begin, Index_type end, Index_type stride,
                   T* min, Index_type *loc,
                   LOOP_BODY loop_body)
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  min_tmp[nthreads];
   Index_type loc_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
       min_tmp[i] = *min ;
       loc_tmp[i] = *loc ;
   }

#pragma omp parallel for
   for ( Index_type ii = begin ; ii < end ; ii += stride ) {
      loop_body( ii, &min_tmp[omp_get_thread_num()],
                     &loc_tmp[omp_get_thread_num()] );
   }

   for ( int i = 1; i < nthreads; ++i ) {
      if ( min_tmp[i] < min_tmp[0] ) {
         min_tmp[0] = min_tmp[i];
         loc_tmp[0] = loc_tmp[i];
      }
   }

   *min = min_tmp[0] ;
   *loc = loc_tmp[0] ;
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for minloc reduction over 
 *         range index set with stride object.
 *
 ******************************************************************************
 */
template <typename T, 
          typename LOOP_BODY>
RAJA_INLINE
void forall_minloc(omp_parallel_for_exec,
                   const RangeStrideISet& is,
                   T* min, Index_type *loc,
                   LOOP_BODY loop_body)
{
   forall_minloc(omp_parallel_for_exec(),
                 is.getBegin(), is.getEnd(), is.getStride(),
                 min, loc,
                 loop_body);
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for maxloc reduction over index range with stride.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_maxloc(omp_parallel_for_exec,
                   Index_type begin, Index_type end, Index_type stride,
                   T* max, Index_type *loc,
                   LOOP_BODY loop_body)
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  max_tmp[nthreads];
   Index_type loc_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
       max_tmp[i] = *max ;
       loc_tmp[i] = *loc ;
   }

#pragma omp parallel for
   for ( Index_type ii = begin ; ii < end ; ii += stride ) {
      loop_body( ii, &max_tmp[omp_get_thread_num()],
                     &loc_tmp[omp_get_thread_num()] );
   }

   for ( int i = 1; i < nthreads; ++i ) {
      if ( max_tmp[i] > max_tmp[0] ) {
         max_tmp[0] = max_tmp[i];
         loc_tmp[0] = loc_tmp[i];
      }
   }

   *max = max_tmp[0] ;
   *loc = loc_tmp[0] ;
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for maxloc reduction over
 *         range index set with stride object.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_maxloc(omp_parallel_for_exec,
                   const RangeStrideISet& is,
                   T* max, Index_type *loc,
                   LOOP_BODY loop_body)
{
   forall_maxloc(omp_parallel_for_exec(),
                 is.getBegin(), is.getEnd(), is.getStride(),
                 max, loc,
                 loop_body);
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for sum reduction over index range with stride.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_sum(omp_parallel_for_exec,
                Index_type begin, Index_type end, Index_type stride,
                T* sum,
                LOOP_BODY loop_body)
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  sum_tmp[nthreads];

   for ( Index_type i = 0; i < nthreads; ++i ) {
      sum_tmp[i] = 0 ;
   }

#pragma omp parallel for
   for ( Index_type ii = begin ; ii < end ; ii += stride ) {
      loop_body( ii, &sum_tmp[omp_get_thread_num()] );
   }

   for ( int i = 0; i < nthreads; ++i ) {
      *sum += sum_tmp[i];
   }
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for sum reduction over
 *         range index set with stride object.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_sum(omp_parallel_for_exec,
                const RangeStrideISet& is,
                T* sum,
                LOOP_BODY loop_body)
{
   forall_sum(omp_parallel_for_exec(),
              is.getBegin(), is.getEnd(), is.getStride(),
              sum,
              loop_body);
}



//
//////////////////////////////////////////////////////////////////////
//
// Function templates that iterate over unstructured index sets.
//
//////////////////////////////////////////////////////////////////////
//

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for iteration over indirection array.
 *
 ******************************************************************************
 */
template <typename LOOP_BODY>
RAJA_INLINE
void forall(omp_parallel_for_exec,
            const Index_type* __restrict__ idx, const Index_type len,
            LOOP_BODY loop_body)
{
#pragma novector
#pragma omp parallel for
   for ( Index_type k = 0 ; k < len ; ++k ) {
      loop_body( idx[k] );
   }
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for iteration over unstructured index set object.
 *
 ******************************************************************************
 */
template <typename LOOP_BODY>
RAJA_INLINE
void forall(omp_parallel_for_exec,
            const UnstructuredISet& is,
            LOOP_BODY loop_body)
{
   const Index_type* __restrict__ idx = is.getIndex();
   const Index_type len = is.getLength();
#pragma novector
#pragma omp parallel for
   for ( Index_type k = 0 ; k < len ; ++k ) {
      loop_body( idx[k] );
   }
}


/*!
 ******************************************************************************
 *
 * \brief  omp parallel for minloc reduction over given indirection array.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_minloc(omp_parallel_for_exec,
                   const Index_type* __restrict__ idx, const Index_type len,
                   T* min, Index_type *loc,
                   LOOP_BODY loop_body)
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  min_tmp[nthreads];
   Index_type loc_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
       min_tmp[i] = *min ;
       loc_tmp[i] = *loc ;
   }

#pragma novector
#pragma omp parallel for
   for ( Index_type k = 0 ; k < len ; ++k ) {
      loop_body( idx[k], &min_tmp[omp_get_thread_num()], 
                         &loc_tmp[omp_get_thread_num()] );
   }

   for ( int i = 1; i < nthreads; ++i ) {
      if ( min_tmp[i] < min_tmp[0] ) {
         min_tmp[0] = min_tmp[i];
         loc_tmp[0] = loc_tmp[i];
      }
   }

   *min = min_tmp[0] ;
   *loc = loc_tmp[0] ;
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for minloc reduction over unstructured index set object.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_minloc(omp_parallel_for_exec,
                   const UnstructuredISet& is,
                   T* min, Index_type *loc,
                   LOOP_BODY loop_body)
{
   forall_minloc(omp_parallel_for_exec(),
                 is.getIndex(), is.getLength(),
                 min, loc,
                 loop_body);
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for maxloc reduction over given indirection array.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_maxloc(omp_parallel_for_exec,
                   const Index_type* __restrict__ idx, const Index_type len,
                   T* max, Index_type *loc,
                   LOOP_BODY loop_body)
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  max_tmp[nthreads];
   Index_type loc_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
       max_tmp[i] = *max ;
       loc_tmp[i] = *loc ;
   }

#pragma novector
#pragma omp parallel for
   for ( Index_type k = 0 ; k < len ; ++k ) {
      loop_body( idx[k], &max_tmp[omp_get_thread_num()],
                         &loc_tmp[omp_get_thread_num()] );
   }

   for ( int i = 1; i < nthreads; ++i ) {
      if ( max_tmp[i] > max_tmp[0] ) {
         max_tmp[0] = max_tmp[i];
         loc_tmp[0] = loc_tmp[i];
      }
   }

   *max = max_tmp[0] ;
   *loc = loc_tmp[0] ;
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for maxloc reduction over unstructured index set object.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_maxloc(omp_parallel_for_exec,
                   const UnstructuredISet& is,
                   T* max, Index_type *loc,
                   LOOP_BODY loop_body)
{
   forall_maxloc(omp_parallel_for_exec(),
                 is.getIndex(), is.getLength(),
                 max, loc,
                 loop_body);
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for sum reduction over given indirection array.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_sum(omp_parallel_for_exec,
                const Index_type* __restrict__ idx, const Index_type len,
                T* sum,
                LOOP_BODY loop_body)
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  sum_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
      sum_tmp[i] = 0 ;
   }

#pragma novector
#pragma omp parallel for
   for ( Index_type k = 0 ; k < len ; ++k ) {
      loop_body( idx[k], &sum_tmp[omp_get_thread_num()] );
   }

   for ( int i = 0; i < nthreads; ++i ) {
      *sum += sum_tmp[i];
   }
}

/*!
 ******************************************************************************
 *
 * \brief  omp parallel for sum reduction over unstructured index set object.
 *
 ******************************************************************************
 */
template <typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_sum(omp_parallel_for_exec,
                const UnstructuredISet& is,
                T* sum,
                LOOP_BODY loop_body)
{
   forall_sum(omp_parallel_for_exec(),
              is.getIndex(), is.getLength(),
              sum,
              loop_body);
}


//
//////////////////////////////////////////////////////////////////////
//
// The following function templates iterate over hybrid index set
// segments using omp execution policies.
//
//////////////////////////////////////////////////////////////////////
//

/*!
 ******************************************************************************
 *
 * \brief  Iterate over hybrid index set segments using omp parallel for 
 *         execution policy and use execution policy template parameter 
 *         for segments.
 *
 ******************************************************************************
 */
template <typename SEG_EXEC_POLICY_T,
          typename LOOP_BODY>
RAJA_INLINE
void forall( std::pair<omp_parallel_for_segit, SEG_EXEC_POLICY_T>,
             const HybridISet& is, LOOP_BODY loop_body )
{
   const int num_seg = is.getNumSegments();

#pragma omp parallel for schedule(dynamic, 1)
   for ( int isi = 0; isi < num_seg; ++isi ) {

      const HybridISet::Segment& seg = is.getSegment(isi);

      switch ( seg.m_type ) {

         case HybridISet::_Range_ : {
            forall(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const RangeISet*>(seg.m_segment)),
               loop_body
            );
            break;
         }

#if 0  // RDH RETHINK
         case HybridISet::_RangeStride_ : {
            forall(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const RangeStrideISet*>(seg.m_segment)),
               loop_body
            );
            break;
         }
#endif

         case HybridISet::_Unstructured_ : {
            forall(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const UnstructuredISet*>(seg.m_segment)),
               loop_body
            );
            break;
         }

         default : {
         }

      }  // switch on segment type

   } // iterate over segments of hybrid index set
}

/*!
 ******************************************************************************
 *
 * \brief  Minloc operation that iterates over hybrid index set segments 
 *         using omp parallel for execution policy and uses execution 
 *         policy template parameter to execute segments.
 *
 ******************************************************************************
 */
template <typename SEG_EXEC_POLICY_T,
          typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_minloc( std::pair<omp_parallel_for_segit, SEG_EXEC_POLICY_T>,
                    const HybridISet& is, 
                    T* min, Index_type *loc,
                    LOOP_BODY loop_body )
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  min_tmp[nthreads];
   Index_type loc_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
       min_tmp[i] = *min ;
       loc_tmp[i] = *loc ;
   }

   const int num_seg = is.getNumSegments();

#pragma omp parallel for 
   for ( int isi = 0; isi < num_seg; ++isi ) {

      const HybridISet::Segment& seg = is.getSegment(isi);

      switch ( seg.m_type ) {

         case HybridISet::_Range_ : {
            forall_minloc(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const RangeISet*>(seg.m_segment)),
               &min_tmp[omp_get_thread_num()], 
               &loc_tmp[omp_get_thread_num()],
               loop_body
            );
            break;
         }

#if 0  // RDH RETHINK
         case HybridISet::_RangeStride_ : {
            forall_minloc(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const RangeStrideISet*>(seg.m_segment)),
               &min_tmp[omp_get_thread_num()], 
               &loc_tmp[omp_get_thread_num()],
               loop_body
            );
            break;
         }
#endif

         case HybridISet::_Unstructured_ : {
            forall_minloc(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const UnstructuredISet*>(seg.m_segment)),
               &min_tmp[omp_get_thread_num()], 
               &loc_tmp[omp_get_thread_num()],
               loop_body
            );
            break;
         }

         default : {
         }

      }  // switch on segment type

   } // iterate over segments of hybrid index set

   for ( int i = 1; i < nthreads; ++i ) {
      if ( min_tmp[i] < min_tmp[0] ) {
         min_tmp[0] = min_tmp[i];
         loc_tmp[0] = loc_tmp[i];
      }
   }

   *min = min_tmp[0] ;
   *loc = loc_tmp[0] ;
}

/*!
 ******************************************************************************
 *
 * \brief  Maxloc operation that iterates over hybrid index set segments 
 *         using omp parallel for execution policy and uses execution 
 *         policy template parameter to execute segments.
 *
 ******************************************************************************
 */
template <typename SEG_EXEC_POLICY_T,
          typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_maxloc( std::pair<omp_parallel_for_segit, SEG_EXEC_POLICY_T>,
                    const HybridISet& is, 
                    T* max, Index_type *loc,
                    LOOP_BODY loop_body )
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  max_tmp[nthreads];
   Index_type loc_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
       max_tmp[i] = *max ;
       loc_tmp[i] = *loc ;
   }

   const int num_seg = is.getNumSegments();

#pragma omp parallel for
   for ( int isi = 0; isi < num_seg; ++isi ) {

      const HybridISet::Segment& seg = is.getSegment(isi);

      switch ( seg.m_type ) {

         case HybridISet::_Range_ : {
            forall_maxloc(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const RangeISet*>(seg.m_segment)),
               &max_tmp[omp_get_thread_num()], 
               &loc_tmp[omp_get_thread_num()],
               loop_body
            );
            break;
         }

#if 0  // RDH RETHINK
         case HybridISet::_RangeStride_ : {
            forall_maxloc(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const RangeStrideISet*>(seg.m_segment)),
               &max_tmp[omp_get_thread_num()], 
               &loc_tmp[omp_get_thread_num()],
               loop_body
            );
            break;
         }
#endif

         case HybridISet::_Unstructured_ : {
            forall_maxloc(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const UnstructuredISet*>(seg.m_segment)),
               &max_tmp[omp_get_thread_num()], 
               &loc_tmp[omp_get_thread_num()],
               loop_body
            );
            break;
         }

         default : {
         }

      }  // switch on segment type

   } // iterate over segments of hybrid index set

   for ( int i = 1; i < nthreads; ++i ) {
      if ( max_tmp[i] > max_tmp[0] ) {
         max_tmp[0] = max_tmp[i];
         loc_tmp[0] = loc_tmp[i];
      }
   }

   *max = max_tmp[0] ;
   *loc = loc_tmp[0] ;
}

/*!
 ******************************************************************************
 *
 * \brief  Sum operation that iterates over hybrid index set segments
 *         using omp parallel for execution policy and uses execution
 *         policy template parameter to execute segments.
 *
 ******************************************************************************
 */
template <typename SEG_EXEC_POLICY_T,
          typename T,
          typename LOOP_BODY>
RAJA_INLINE
void forall_sum( std::pair<omp_parallel_for_segit, SEG_EXEC_POLICY_T>,
                 const HybridISet& is,
                 T* sum,
                 LOOP_BODY loop_body )
{
   const int nthreads = omp_get_max_threads();

   /* Should we align these temps to coherence boundaries? */
   T  sum_tmp[nthreads];

   for ( int i = 0; i < nthreads; ++i ) {
       sum_tmp[i] = 0 ;
   }

   const int num_seg = is.getNumSegments();

#pragma omp parallel for
   for ( int isi = 0; isi < num_seg; ++isi ) {

      const HybridISet::Segment& seg = is.getSegment(isi);

      switch ( seg.m_type ) {

         case HybridISet::_Range_ : {
            forall_sum(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const RangeISet*>(seg.m_segment)),
               &sum_tmp[omp_get_thread_num()],
               loop_body
            );
            break;
         }

#if 0  // RDH RETHINK
         case HybridISet::_RangeStride_ : {
            forall_sum(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const RangeStrideISet*>(seg.m_segment)),
               &sum_tmp[omp_get_thread_num()],
               loop_body
            );
            break;
         }
#endif

         case HybridISet::_Unstructured_ : {
            forall_sum(
               SEG_EXEC_POLICY_T(),
               *(static_cast<const UnstructuredISet*>(seg.m_segment)),
               &sum_tmp[omp_get_thread_num()],
               loop_body
            );
            break;
         }

         default : {
         }

      }  // switch on segment type

   } // iterate over segments of hybrid index set

   for ( int i = 0; i < nthreads; ++i ) {
      *sum += sum_tmp[i];
   }
}


}  // closing brace for namespace statement

#endif  // closing endif for header file include guard