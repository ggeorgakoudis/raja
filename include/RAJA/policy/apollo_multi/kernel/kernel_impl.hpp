#ifndef RAJA_policy_apollo_multi_kernel_impl_HPP
#define RAJA_policy_apollo_multi_kernel_impl_HPP

#include "RAJA/policy/apollo_multi/kernel.hpp"
//#include "RAJA/policy/cuda/kernel/CudaKernel.hpp"

#include "apollo/Apollo.h"
#include "apollo/Region.h"

namespace RAJA 
{

namespace policy
{

namespace apollo_multi
{

template<typename KernelPolicy>
struct PreLaunchKernel {
  static void exec(Apollo::Region *region, Apollo::RegionContext *context)
  {
    //std::cout << "PreLaunch *NOT* CudaKernelAsync\n";
  }
};

template<typename KernelPolicy>
struct PostLaunchKernel {
  static void exec(Apollo::Region *region, Apollo::RegionContext *context)
  {
    region->end(context);
    //std::cout << "PostLaunch *NOT* CudaKernelAsync\n";
  }
};


template<typename... Args>
struct PreLaunchKernel<RAJA::statement::CudaKernelAsync<Args...>> {
  static void exec(Apollo::Region *region, Apollo::RegionContext *context)
  {
    using callback_t = RAJA::apollo_cuda::ApolloCallbackDataPool::callback_t;
    callback_t *cbdata =
        reinterpret_cast<callback_t *>(region->callback_pool->get());
    context->isDoneCallback =
        RAJA::apollo_cuda::ApolloCallbackHelper::isDoneCallback;
    context->callback_arg = cbdata;

    //std::cout << "PreLaunch CudaKernelAsync\n";
    // TODO: how to handle multiple streams?
    cudaEventRecord(cbdata->start, 0);
  }
};

template<typename... Args>
struct PostLaunchKernel<RAJA::statement::CudaKernelAsync<Args...>> {
  static void exec(Apollo::Region *region, Apollo::RegionContext *context)
  {
    using callback_t = RAJA::apollo_cuda::ApolloCallbackDataPool::callback_t;
    callback_t *cbdata = reinterpret_cast<callback_t *>(context->callback_arg);
    // TODO: how to handle multiple streams?
    cudaEventRecord(cbdata->stop, 0);
    region->end(context);

    //std::cout << "PostLaunch CudaKernelAsync\n";
  }
};

template <camp::idx_t idx,
          camp::idx_t num_policies,
          typename KernelPolicyList,
          typename SegmentTuple,
          typename... Bodies>
struct KernelPolicyGeneratorSingle {
  static RAJA_INLINE void generate(int policy,
                                   Apollo::Region *region,
                                   Apollo::RegionContext *context,
                                   SegmentTuple &&segments,
                                   Bodies &&... bodies)
  {
    // Get execution policy.
    using ExecutionPolicy = camp::at_v<KernelPolicyList, idx>;
    // Get first stmt from the statement list to identify CUDA async execution.
    using stmt_t = camp::at_v<ExecutionPolicy, 0>;

    //std::cout << "KernelPolicyGeneratorSingle p:" << policy << " r:" << region->name << " ctx:" << context->idx << "\n";
    if (policy == idx) {
      // generate policy variant, calls top-level kernel pattern.
      PreLaunchKernel<stmt_t>::exec(region, context);

      RAJA::kernel<ExecutionPolicy>(std::forward<SegmentTuple>(segments),
                           std::forward<Bodies>(bodies)...);

      PostLaunchKernel<stmt_t>::exec(region, context);
    } else
      KernelPolicyGeneratorSingle<idx + 1, num_policies, KernelPolicyList, SegmentTuple, Bodies...>::
          generate(policy,
                   region,
                   context,
                   std::forward<SegmentTuple>(segments),
                   std::forward<Bodies>(bodies)...);
  }
};

template <camp::idx_t num_policies,
          typename KernelPolicyList,
          typename SegmentTuple,
          typename... Bodies>
struct KernelPolicyGeneratorSingle<num_policies,
                             num_policies,
                             KernelPolicyList,
                             SegmentTuple,
                             Bodies...> {
  static RAJA_INLINE void generate(int policy,
                                   Apollo::Region *region,
                                   Apollo::RegionContext *context,
                                   SegmentTuple &&segments,
                                   Bodies &&... bodies)
  {
  }
};

template <typename KernelPolicyList, typename SegmentTuple, typename... Bodies>
static RAJA_INLINE void KernelPolicyGenerator(int policy,
                                        Apollo::Region *region,
                                        Apollo::RegionContext *context,
                                        SegmentTuple &&segments,
                                        Bodies &&...bodies)
{
  //std::cout << "KernelPolicyGenerator p:" << policy << " r:" << region->name << " ctx:" << context->idx << "\n";
  KernelPolicyGeneratorSingle<0,
                        camp::size<KernelPolicyList>::value,
                        KernelPolicyList,
                        SegmentTuple,
                        Bodies...>::generate(policy,
                                        region,
                                        context,
                                        std::forward<SegmentTuple>(segments),
                                        std::forward<Bodies>(bodies)...);
}


/* TEST STUFF */
template <camp::idx_t idx, camp::idx_t num_tuples, typename SegmentTuple>
struct FeatureGeneratorSingle {
  static void generate(SegmentTuple segments, std::vector<float> &features) {
    features.push_back(camp::get<idx>(segments).size());

    FeatureGeneratorSingle<idx+1, num_tuples, SegmentTuple>::generate(segments, features);

    return;
  }
};

template <camp::idx_t num_tuples, typename SegmentTuple>
struct FeatureGeneratorSingle<num_tuples, num_tuples, SegmentTuple> {
  static void generate(SegmentTuple segments, std::vector<float> &features) {
    return;
  }
};

template <typename SegmentTuple>
struct FeatureGenerator {
  static void generate(SegmentTuple segments, std::vector<float> &features) {
    FeatureGeneratorSingle<0, camp::tuple_size<SegmentTuple>::value, SegmentTuple>::generate(segments, features);
  }
};

/* END OF TEST STUFF */

template <typename KernelPolicyList, typename SegmentTuple, typename... Bodies>
RAJA_INLINE void kernel_impl(const ApolloMultiKernelPolicy<KernelPolicyList> &p,
                             SegmentTuple &&segments,
                             Bodies &&...bodies)
{
  static Apollo *apollo = Apollo::instance();
  static Apollo::Region *apolloRegion = nullptr;
  static int policy_index = 0;
  // TODO: Move the callback data pool to generic apollo header
  // TODO: Fix pool for kernels
  static RAJA::apollo_cuda::ApolloCallbackDataPool *callback_pool = nullptr;
  if (apolloRegion == nullptr) {
    std::string code_location = apollo->getCallpathOffset();
    callback_pool = new RAJA::apollo_cuda::ApolloCallbackDataPool(64, 64);
    apolloRegion =
        new Apollo::Region(/* num features */ camp::tuple_size<SegmentTuple>::value,
                           /* region id */ code_location.c_str(),
                           /* num policies */ camp::size<KernelPolicyList>::value,
                           /* callback pool */ callback_pool
        );
  }

  std::vector<float> features;
  FeatureGenerator<SegmentTuple>::generate(segments, features);

  /*std::cout << "features: [ ";
  for(auto e : features)
    std::cout << e << ", ";
  std::cout << " ]\n";*/
  Apollo::RegionContext *context = apolloRegion->begin(features);

  policy_index = apolloRegion->getPolicyIndex(context);
  //std::cout << "policy_index " << policy_index << std::endl;  // ggout

  KernelPolicyGenerator<KernelPolicyList, SegmentTuple, Bodies...>(policy_index,
                                              apolloRegion,
                                              context,
                                              std::forward<SegmentTuple>(segments),
                                              std::forward<Bodies>(bodies)...);
}

}  // namespace apollo_multi

}  // namespace policy

}  // namespace RAJA

#endif