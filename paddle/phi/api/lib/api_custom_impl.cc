/* Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/phi/api/lib/api_custom_impl.h"

#include "paddle/phi/api/lib/api_gen_utils.h"
#include "paddle/phi/api/lib/data_transform.h"
#include "paddle/phi/api/lib/kernel_dispatch.h"
#include "paddle/phi/api/lib/utils/storage.h"
#include "paddle/phi/common/type_traits.h"
#include "paddle/phi/core/compat/convert_utils.h"
#include "paddle/phi/core/kernel_registry.h"
#include "paddle/phi/core/meta_tensor.h"
#include "paddle/phi/infermeta/backward.h"
#include "paddle/phi/infermeta/binary.h"
#include "paddle/phi/infermeta/multiary.h"
#include "paddle/phi/infermeta/nullary.h"
#include "paddle/phi/infermeta/unary.h"

#include "glog/logging.h"

namespace paddle {
namespace experimental {

////////////////// Forward api impls //////////////////////

Tensor conv2d_impl(const Tensor& input,
                   const Tensor& filter,
                   const std::vector<int>& strides,
                   const std::vector<int>& paddings,
                   const std::string& paddding_algorithm,
                   int groups,
                   const std::vector<int>& dilations,
                   const std::string& data_format,
                   bool use_addto,
                   int workspace_size_MB,
                   bool exhaustive_search) {
  Backend kernel_backend = Backend::UNDEFINED;
  DataLayout kernel_layout = DataLayout::UNDEFINED;
  DataType kernel_data_type = DataType::UNDEFINED;

  kernel_data_type = ParseDataType(input);

  if (kernel_backend == Backend::UNDEFINED ||
      kernel_layout == DataLayout::UNDEFINED ||
      kernel_data_type == DataType::UNDEFINED) {
    auto kernel_key_set = ParseKernelKeyByInputArgs(input, filter);
    auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();
    if (kernel_backend == Backend::UNDEFINED) {
      kernel_backend = kernel_key.backend();
    }
    if (kernel_layout == DataLayout::UNDEFINED) {
      kernel_layout = kernel_key.layout();
    }
    if (kernel_data_type == DataType::UNDEFINED) {
      kernel_data_type = kernel_key.dtype();
    }
  }

  VLOG(6) << "conv2d API kernel key: [" << kernel_backend << ", "
          << kernel_layout << ", " << kernel_data_type << "]";
  const auto& kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "conv2d", {kernel_backend, kernel_layout, kernel_data_type}, true);
  VLOG(6) << "conv2d API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_backend);

  phi::TensorArgDef args0 = kernel.InputAt(0);
  phi::TensorArgDef args1 = kernel.InputAt(1);
  if (kernel_backend == Backend::GPU) {
    args0.backend = Backend::GPU;
    args1.backend = Backend::GPU;
  }

  auto input_input = PrepareData(input, args0, {});
  auto input_filter = PrepareData(filter, args1, {});

  Tensor api_output;
  auto kernel_out = SetKernelOutput(kernel_backend, &api_output);
  phi::MetaTensor meta_out(kernel_out);

  phi::ConvInferMeta(MakeMetaTensor(*input_input),
                     MakeMetaTensor(*input_filter),
                     strides,
                     paddings,
                     paddding_algorithm,
                     groups,
                     dilations,
                     data_format,
                     use_addto,
                     workspace_size_MB,
                     exhaustive_search,
                     &meta_out);

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const phi::DenseTensor&,
                                    const phi::DenseTensor&,
                                    const std::vector<int>&,
                                    const std::vector<int>&,
                                    const std::string&,
                                    int,
                                    const std::vector<int>&,
                                    const std::string&,
                                    bool,
                                    int,
                                    bool,
                                    phi::DenseTensor*);
  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();

  {
    (*kernel_fn)(*dev_ctx,
                 *input_input,
                 *input_filter,
                 strides,
                 paddings,
                 paddding_algorithm,
                 groups,
                 dilations,
                 data_format,
                 use_addto,
                 workspace_size_MB,
                 exhaustive_search,
                 kernel_out);
  }

  return api_output;
}

std::vector<std::vector<Tensor>> conv2d_grad_impl(
    const Tensor& input,
    const Tensor& filter,
    const Tensor& out_grad,
    const std::vector<int>& strides,
    const std::vector<int>& paddings,
    const std::string& paddding_algorithm,
    int groups,
    const std::vector<int>& dilations,
    const std::string& data_format,
    bool use_addto,
    int workspace_size_MB,
    bool exhaustive_search) {
  Backend kernel_backend = Backend::UNDEFINED;
  DataLayout kernel_layout = DataLayout::UNDEFINED;
  DataType kernel_data_type = DataType::UNDEFINED;

  if (kernel_backend == Backend::UNDEFINED ||
      kernel_layout == DataLayout::UNDEFINED ||
      kernel_data_type == DataType::UNDEFINED) {
    auto kernel_key_set = ParseKernelKeyByInputArgs(input, filter, out_grad);
    auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();
    if (kernel_backend == Backend::UNDEFINED) {
      kernel_backend = kernel_key.backend();
    }
    if (kernel_layout == DataLayout::UNDEFINED) {
      kernel_layout = kernel_key.layout();
    }
    if (kernel_data_type == DataType::UNDEFINED) {
      kernel_data_type = kernel_key.dtype();
    }
  }

  VLOG(6) << "conv2d_grad API kernel key: [" << kernel_backend << ", "
          << kernel_layout << ", " << kernel_data_type << "]";
  const auto& kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "conv2d_grad", {kernel_backend, kernel_layout, kernel_data_type}, true);
  VLOG(6) << "conv2d_grad API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_backend);

  phi::TensorArgDef args0 = kernel.InputAt(0);
  phi::TensorArgDef args1 = kernel.InputAt(1);
  phi::TensorArgDef args2 = kernel.InputAt(2);
  if (kernel_backend == Backend::GPU) {
    args0.backend = Backend::GPU;
    args1.backend = Backend::GPU;
    args2.backend = Backend::GPU;
  }

  auto input_input = PrepareData(input, args0, {});
  auto input_filter = PrepareData(filter, args1, {});
  auto input_out_grad = PrepareData(out_grad, args2, {});

  std::vector<std::vector<Tensor>> api_output(2);
  api_output[0].emplace_back();
  auto kernel_out_0 = SetKernelOutput(kernel_backend, &api_output[0][0]);
  api_output[1].emplace_back();
  auto kernel_out_1 = SetKernelOutput(kernel_backend, &api_output[1][0]);
  phi::MetaTensor meta_out_0(kernel_out_0);
  phi::MetaTensor meta_out_1(kernel_out_1);

  phi::GeneralBinaryGradInferMeta(MakeMetaTensor(*input_input),
                                  MakeMetaTensor(*input_filter),
                                  &meta_out_0,
                                  &meta_out_1);

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const phi::DenseTensor&,
                                    const phi::DenseTensor&,
                                    const phi::DenseTensor&,
                                    const std::vector<int>&,
                                    const std::vector<int>&,
                                    const std::string&,
                                    int,
                                    const std::vector<int>&,
                                    const std::string&,
                                    bool,
                                    int,
                                    bool,
                                    phi::DenseTensor*,
                                    phi::DenseTensor*);
  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();

  {
    (*kernel_fn)(*dev_ctx,
                 *input_input,
                 *input_filter,
                 *input_out_grad,
                 strides,
                 paddings,
                 paddding_algorithm,
                 groups,
                 dilations,
                 data_format,
                 use_addto,
                 workspace_size_MB,
                 exhaustive_search,
                 kernel_out_0,
                 kernel_out_1);
  }

  return api_output;
}

Tensor copy_to_impl(const Tensor& x, Place place, bool blocking) {
  auto kernel_key_set = ParseKernelKeyByInputArgs(x);
  kernel_key_set.backend_set =
      kernel_key_set.backend_set | BackendSet(phi::TransToPhiBackend(place));
  auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();
  auto kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "copy", kernel_key);

  VLOG(6) << "copy API kernel key: " << kernel_key;
  VLOG(6) << "copy API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_key.backend());

  auto dense_x = TensorToDenseTensor(x);

  Tensor out;
  auto kernel_out = SetKernelOutput(kernel_key.backend(), &out);
  phi::MetaTensor meta_out(kernel_out);
  phi::UnchangedInferMeta(*dense_x, &meta_out);

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const phi::DenseTensor&,
                                    phi::Place,
                                    bool,
                                    phi::DenseTensor*);

  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();

  (*kernel_fn)(*dev_ctx, *dense_x, place, blocking, kernel_out);

  return out;
}

std::vector<Tensor> split_impl(const Tensor& x,
                               const IntArray& num_or_sections,
                               const Scalar& axis) {
  auto kernel_key_set = ParseKernelKeyByInputArgs(x);
  auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();

  Backend kernel_backend = kernel_key.backend();
  DataLayout kernel_layout = kernel_key.layout();
  DataType kernel_data_type = kernel_key.dtype();

  auto kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "split", {kernel_backend, kernel_layout, kernel_data_type});
  VLOG(6) << "split API kernel key: [" << kernel_backend << ", "
          << kernel_layout << ", " << kernel_data_type << "]";
  VLOG(6) << "split API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_backend);

  auto dense_x = PrepareData(x, kernel.InputAt(0), {});

  // Calculate the number of out tensors
  size_t out_number;
  if (num_or_sections.GetData().size() == 1) {
    out_number = num_or_sections.GetData()[0];
  } else {
    out_number = num_or_sections.GetData().size();
  }

  std::vector<Tensor> out;
  auto dense_outs = SetKernelOutput(out_number, kernel_backend, &out);
  std::vector<phi::MetaTensor> meta_outs;
  meta_outs.reserve(out_number);
  std::vector<phi::MetaTensor*> meta_out_ptrs;
  meta_out_ptrs.reserve(out_number);
  for (size_t i = 0; i < out_number; ++i) {
    meta_outs.push_back(dense_outs[i]);
    meta_out_ptrs.push_back(&meta_outs.back());
  }

  phi::SplitInferMeta(
      MakeMetaTensor(*dense_x), num_or_sections, axis, meta_out_ptrs);

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const phi::DenseTensor&,
                                    const phi::IntArray&,
                                    const phi::Scalar&,
                                    std::vector<phi::DenseTensor*>&);
  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();
  (*kernel_fn)(*dev_ctx,
               *dense_x,
               phi::IntArray(num_or_sections),
               phi::Scalar(axis),
               dense_outs);

  return out;
}

std::tuple<Tensor, Tensor, Tensor> momentum_impl(
    const Tensor& param,
    const Tensor& grad,
    const Tensor& velocity,
    const Tensor& learning_rate,
    paddle::optional<const Tensor&> master_param,
    float mu,
    bool use_nesterov,
    const std::string& regularization_method,
    float regularization_coeff,
    bool multi_precision,
    float rescale_grad) {
  Backend kernel_backend = Backend::UNDEFINED;
  DataLayout kernel_layout = DataLayout::UNDEFINED;
  DataType kernel_data_type = DataType::UNDEFINED;
  if (kernel_backend == Backend::UNDEFINED ||
      kernel_layout == DataLayout::UNDEFINED ||
      kernel_data_type == DataType::UNDEFINED) {
    auto kernel_key_set = ParseKernelKeyByInputArgs(param);
    auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();
    if (kernel_backend == Backend::UNDEFINED) {
      kernel_backend = kernel_key.backend();
    }
    if (kernel_layout == DataLayout::UNDEFINED) {
      kernel_layout = kernel_key.layout();
    }
    if (kernel_data_type == DataType::UNDEFINED) {
      kernel_data_type = kernel_key.dtype();
    }
  }
  std::string kernel_name = "momentum";
  if (grad.is_selected_rows()) {
    kernel_name = "momentum_dense_param_sparse_grad";
  }
  const auto& kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      kernel_name, {kernel_backend, kernel_layout, kernel_data_type});
  VLOG(6) << kernel_name << " API kernel key: [" << kernel_backend << ", "
          << kernel_layout << ", " << kernel_data_type << "]";
  VLOG(6) << kernel_name << " API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_backend);

  auto input_param = PrepareData(param, kernel.InputAt(0), {});
  auto input_grad = PrepareData(grad, kernel.InputAt(1), {});
  auto input_velocity = PrepareData(velocity, kernel.InputAt(2), {});
  auto input_learning_rate = PrepareData(learning_rate, kernel.InputAt(3), {});
  paddle::optional<const phi::DenseTensor&> input_master_param(paddle::none);
  auto input_master_param_ptr =
      PrepareData(master_param, kernel.InputAt(4), {});

  std::tuple<Tensor, Tensor, Tensor> api_output;
  auto kernel_out_0 = input_param.get();
  auto kernel_out_1 = input_velocity.get();
  phi::DenseTensor* kernel_out_2 = nullptr;
  if (input_master_param_ptr) {
    input_master_param =
        paddle::make_optional<const phi::DenseTensor&>(*input_master_param_ptr);
    kernel_out_2 =
        paddle::make_optional<phi::DenseTensor&>(*input_master_param_ptr)
            .get_ptr();
  }

  paddle::optional<const phi::MetaTensor&> input_meta_ref_master_param(
      paddle::none);
  phi::DenseTensor dt;
  phi::MetaTensor input_meta_tmp_master_param(dt);
  if (input_master_param_ptr) {
    input_meta_tmp_master_param.set_dtype(input_master_param_ptr->dtype());
    input_meta_tmp_master_param.set_dims(input_master_param_ptr->dims());
    input_meta_tmp_master_param.set_layout(input_master_param_ptr->layout());
    input_meta_ref_master_param = input_meta_tmp_master_param;
  }
  phi::MetaTensor meta_out_0(kernel_out_0);
  phi::MetaTensor meta_out_1(kernel_out_1);
  if (kernel_out_2) {
    phi::MetaTensor meta_out_2(kernel_out_2);
    phi::MomentumInferMeta(MakeMetaTensor(*input_param),
                           MakeMetaTensor(*input_grad),
                           MakeMetaTensor(*input_velocity),
                           MakeMetaTensor(*input_learning_rate),
                           input_meta_ref_master_param,
                           mu,
                           use_nesterov,
                           regularization_method,
                           regularization_coeff,
                           multi_precision,
                           rescale_grad,
                           &meta_out_0,
                           &meta_out_1,
                           &meta_out_2);
  } else {
    phi::MomentumInferMeta(MakeMetaTensor(*input_param),
                           MakeMetaTensor(*input_grad),
                           MakeMetaTensor(*input_velocity),
                           MakeMetaTensor(*input_learning_rate),
                           input_meta_ref_master_param,
                           mu,
                           use_nesterov,
                           regularization_method,
                           regularization_coeff,
                           multi_precision,
                           rescale_grad,
                           &meta_out_0,
                           &meta_out_1,
                           nullptr);
  }

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const phi::DenseTensor&,
                                    const phi::DenseTensor&,
                                    const phi::DenseTensor&,
                                    const phi::DenseTensor&,
                                    paddle::optional<const phi::DenseTensor&>,
                                    float,
                                    bool,
                                    const std::string&,
                                    float,
                                    bool,
                                    float,
                                    phi::DenseTensor*,
                                    phi::DenseTensor*,
                                    phi::DenseTensor*);
  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();

  (*kernel_fn)(*dev_ctx,
               *input_param,
               *input_grad,
               *input_velocity,
               *input_learning_rate,
               input_master_param,
               mu,
               use_nesterov,
               regularization_method,
               regularization_coeff,
               multi_precision,
               rescale_grad,
               kernel_out_0,
               kernel_out_1,
               kernel_out_2);

  return api_output;
}

////////////////// Backward(grad) api impls //////////////////////

// TODO(chenweihang):  the original sum grad op can support higher-level
// differentiation,
// but if we use this impl, it will not support. We need to be able to reuse
// the autograd API here, which is not yet implemented
// TODO(chenweihang): we should support call generated api in custom api impl
std::vector<Tensor> add_n_grad_impl(const std::vector<Tensor>& x,
                                    const Tensor& out_grad) {
  auto kernel_key_set = ParseKernelKeyByInputArgs(out_grad);
  auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();

  Backend kernel_backend = kernel_key.backend();
  DataLayout kernel_layout = kernel_key.layout();
  DataType kernel_data_type = kernel_key.dtype();

  auto kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "scale", {kernel_backend, kernel_layout, kernel_data_type});
  VLOG(6) << "add_n_grad API kernel key: [" << kernel_backend << ", "
          << kernel_layout << ", " << kernel_data_type << "]";
  VLOG(6) << "add_n_grad API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_backend);

  auto dense_out_grad = PrepareData(out_grad, kernel.InputAt(0), {});

  size_t out_number = x.size();
  std::vector<Tensor> x_grad;
  auto dense_x_grad = SetKernelOutput(out_number, kernel_backend, &x_grad);

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const phi::DenseTensor&,
                                    const phi::Scalar&,
                                    float,
                                    bool,
                                    phi::DenseTensor*);
  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();

  for (auto* dense_x_grad_t : dense_x_grad) {
    phi::MetaTensor meta_out(dense_x_grad_t);
    phi::UnchangedInferMeta(MakeMetaTensor(*dense_out_grad), &meta_out);
    (*kernel_fn)(
        *dev_ctx, *dense_out_grad, phi::Scalar(1.0), 0.0, true, dense_x_grad_t);
  }

  return x_grad;
}

std::tuple<Tensor, Tensor, Tensor, Tensor, Tensor, Tensor> batch_norm_impl(
    const Tensor& x,
    const Tensor& scale,
    const Tensor& bias,
    const Tensor& mean,
    const Tensor& variance,
    float momentum,
    float epsilon,
    const std::string& data_layout,
    bool is_test,
    bool use_global_stats,
    bool trainable_statistics,
    bool fuse_with_relu) {
  Backend kernel_backend = Backend::UNDEFINED;
  DataLayout kernel_layout = DataLayout::UNDEFINED;
  DataType kernel_data_type = DataType::UNDEFINED;

  kernel_data_type = ParseDataType(x);

  if (kernel_backend == Backend::UNDEFINED ||
      kernel_layout == DataLayout::UNDEFINED ||
      kernel_data_type == DataType::UNDEFINED) {
    auto kernel_key_set = ParseKernelKeyByInputArgs(x);
    auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();
    if (kernel_backend == Backend::UNDEFINED) {
      kernel_backend = kernel_key.backend();
    }
    if (kernel_layout == DataLayout::UNDEFINED) {
      kernel_layout = kernel_key.layout();
    }
    if (kernel_data_type == DataType::UNDEFINED) {
      kernel_data_type = kernel_key.dtype();
    }
  }

  const auto& kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "batch_norm", {kernel_backend, kernel_layout, kernel_data_type});
  VLOG(6) << "batch_norm API kernel key: [" << kernel_backend << ", "
          << kernel_layout << ", " << kernel_data_type << "]";
  VLOG(6) << "batch_norm API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_backend);

  auto input_x = PrepareData(x, kernel.InputAt(0), {});
  auto input_scale = PrepareData(scale, kernel.InputAt(1), {});
  auto input_bias = PrepareData(bias, kernel.InputAt(2), {});
  auto input_mean = PrepareData(mean, kernel.InputAt(3), {});
  auto input_variance = PrepareData(variance, kernel.InputAt(4), {});

  std::tuple<Tensor, Tensor, Tensor, Tensor, Tensor, Tensor> api_output;
  auto kernel_out_0 = SetKernelOutput(kernel_backend, &std::get<0>(api_output));
  std::get<1>(api_output).set_impl(mean.impl());
  std::get<2>(api_output).set_impl(variance.impl());
  auto kernel_out_1 = SetKernelOutput(kernel_backend, &std::get<1>(api_output));
  auto kernel_out_2 = SetKernelOutput(kernel_backend, &std::get<2>(api_output));
  auto kernel_out_3 = SetKernelOutput(kernel_backend, &std::get<3>(api_output));
  auto kernel_out_4 = SetKernelOutput(kernel_backend, &std::get<4>(api_output));
  auto kernel_out_5 = SetKernelOutput(kernel_backend, &std::get<5>(api_output));
  phi::MetaTensor meta_out_0(kernel_out_0);
  phi::MetaTensor meta_out_1(kernel_out_1);
  phi::MetaTensor meta_out_2(kernel_out_2);
  phi::MetaTensor meta_out_3(kernel_out_3);
  phi::MetaTensor meta_out_4(kernel_out_4);
  phi::MetaTensor meta_out_5(kernel_out_5);

  phi::BatchNormInferMeta(MakeMetaTensor(*input_x),
                          MakeMetaTensor(*input_scale),
                          MakeMetaTensor(*input_bias),
                          MakeMetaTensor(*input_mean),
                          MakeMetaTensor(*input_variance),
                          momentum,
                          epsilon,
                          data_layout,
                          is_test,
                          use_global_stats,
                          trainable_statistics,
                          fuse_with_relu,
                          &meta_out_0,
                          &meta_out_1,
                          &meta_out_2,
                          &meta_out_3,
                          &meta_out_4,
                          &meta_out_5);

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const phi::DenseTensor&,
                                    const phi::DenseTensor&,
                                    const phi::DenseTensor&,
                                    const phi::DenseTensor&,
                                    const phi::DenseTensor&,
                                    float,
                                    float,
                                    const std::string&,
                                    bool,
                                    bool,
                                    bool,
                                    bool,
                                    phi::DenseTensor*,
                                    phi::DenseTensor*,
                                    phi::DenseTensor*,
                                    phi::DenseTensor*,
                                    phi::DenseTensor*,
                                    phi::DenseTensor*);
  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();
  {
    (*kernel_fn)(*dev_ctx,
                 *input_x,
                 *input_scale,
                 *input_bias,
                 *input_mean,
                 *input_variance,
                 momentum,
                 epsilon,
                 data_layout,
                 is_test,
                 use_global_stats,
                 trainable_statistics,
                 fuse_with_relu,
                 kernel_out_0,
                 kernel_out_1,
                 kernel_out_2,
                 kernel_out_3,
                 kernel_out_4,
                 kernel_out_5);
  }

  return api_output;
}

std::vector<Tensor> concat_grad_impl(const std::vector<Tensor>& x,
                                     const Tensor& out_grad,
                                     const Scalar& axis) {
  auto kernel_key_set = ParseKernelKeyByInputArgs(out_grad);
  auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();

  Backend kernel_backend = kernel_key.backend();
  DataLayout kernel_layout = kernel_key.layout();
  DataType kernel_data_type = kernel_key.dtype();

  auto kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "concat_grad", {kernel_backend, kernel_layout, kernel_data_type});
  VLOG(6) << "concat_grad API kernel key: [" << kernel_backend << ", "
          << kernel_layout << ", " << kernel_data_type << "]";
  VLOG(6) << "concat_grad API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_backend);

  // std::unique_ptr<std::vector<phi::DenseTensor>>
  auto dense_x = PrepareData(x, kernel.InputAt(0), {});
  auto dense_out_grad = PrepareData(out_grad, kernel.InputAt(1), {});

  // Calculate the number of out tensors
  size_t out_number = x.size();
  std::vector<Tensor> x_grad;
  auto dense_x_grad = SetKernelOutput(out_number, kernel_backend, &x_grad);

  std::vector<phi::MetaTensor> meta_x;
  meta_x.reserve(x.size());
  std::vector<phi::MetaTensor*> meta_x_ptrs;
  meta_x_ptrs.reserve(x.size());
  for (const auto& t : *dense_x) {
    meta_x.push_back(t);
    meta_x_ptrs.push_back(&meta_x.back());
  }

  std::vector<phi::MetaTensor> meta_x_grad;
  meta_x_grad.reserve(x.size());
  std::vector<phi::MetaTensor*> meta_x_grad_ptrs;
  meta_x_grad_ptrs.reserve(x.size());
  for (size_t i = 0; i < out_number; ++i) {
    meta_x_grad.push_back(*dense_x_grad[i]);
    meta_x_grad_ptrs.push_back(&meta_x_grad.back());
  }

  phi::UnchangedMultiInferMeta(meta_x_ptrs, meta_x_grad_ptrs);

  std::vector<const phi::DenseTensor*> dense_x_ptr;
  dense_x_ptr.reserve(x.size());
  for (const auto& t : *dense_x) {
    dense_x_ptr.push_back(&t);
  }

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const std::vector<const phi::DenseTensor*>&,
                                    const phi::DenseTensor&,
                                    const phi::Scalar&,
                                    std::vector<phi::DenseTensor*>);
  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();
  (*kernel_fn)(
      *dev_ctx, dense_x_ptr, *dense_out_grad, phi::Scalar(axis), dense_x_grad);

  return x_grad;
}

Tensor imag_grad_impl(const Tensor& out_grad) {
  phi::KernelKey kernel_key{ParseBackend(out_grad),
                            out_grad.layout(),
                            phi::dtype::ToComplex(out_grad.dtype())};
  auto kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "imag_grad", kernel_key);

  VLOG(6) << "imag_grad API kernel key: " << kernel_key;
  VLOG(6) << "imag_grad API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_key.backend());

  auto dense_out_grad = TensorToDenseTensor(out_grad);

  Tensor out;
  auto kernel_out = SetKernelOutput(kernel_key.backend(), &out);
  phi::MetaTensor meta_out(kernel_out);
  phi::RealAndImagGradInferMeta(*dense_out_grad, &meta_out);

  using kernel_signature = void (*)(
      const phi::DeviceContext&, const phi::DenseTensor&, phi::DenseTensor*);

  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();
  (*kernel_fn)(*dev_ctx, *dense_out_grad, kernel_out);

  return out;
}

Tensor real_grad_impl(const Tensor& out_grad) {
  phi::KernelKey kernel_key{ParseBackend(out_grad),
                            out_grad.layout(),
                            phi::dtype::ToComplex(out_grad.dtype())};
  auto kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "real_grad", kernel_key);

  VLOG(6) << "real_grad API kernel key: " << kernel_key;
  VLOG(6) << "real_grad API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_key.backend());

  auto dense_out_grad = TensorToDenseTensor(out_grad);

  Tensor out;
  auto kernel_out = SetKernelOutput(kernel_key.backend(), &out);
  phi::MetaTensor meta_out(kernel_out);
  phi::RealAndImagGradInferMeta(*dense_out_grad, &meta_out);

  using kernel_signature = void (*)(
      const phi::DeviceContext&, const phi::DenseTensor&, phi::DenseTensor*);

  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();
  (*kernel_fn)(*dev_ctx, *dense_out_grad, kernel_out);

  return out;
}

std::vector<Tensor> stack_grad_impl(const std::vector<Tensor>& x,
                                    const Tensor& out_grad,
                                    int axis) {
  auto kernel_key_set = ParseKernelKeyByInputArgs(out_grad);
  auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();

  Backend kernel_backend = kernel_key.backend();
  DataLayout kernel_layout = kernel_key.layout();
  DataType kernel_data_type = kernel_key.dtype();

  auto kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "stack_grad", {kernel_backend, kernel_layout, kernel_data_type});
  VLOG(6) << "stack_grad API kernel key: [" << kernel_backend << ", "
          << kernel_layout << ", " << kernel_data_type << "]";
  VLOG(6) << "stack_grad API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_backend);

  auto dense_out_grad = PrepareData(out_grad, kernel.InputAt(0), {});

  size_t out_number = x.size();
  std::vector<Tensor> x_grad;
  auto dense_x_grad = SetKernelOutput(out_number, kernel_backend, &x_grad);
  std::vector<phi::MetaTensor> meta_x_grad;
  meta_x_grad.reserve(out_number);
  std::vector<phi::MetaTensor*> meta_x_grad_ptrs;
  meta_x_grad_ptrs.reserve(out_number);
  for (size_t i = 0; i < out_number; ++i) {
    meta_x_grad.push_back(dense_x_grad[i]);
    meta_x_grad_ptrs.push_back(&meta_x_grad.back());
  }

  phi::StackGradInferMeta(
      MakeMetaTensor(*dense_out_grad), axis, meta_x_grad_ptrs);

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const phi::DenseTensor&,
                                    int axis,
                                    std::vector<phi::DenseTensor*>);
  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();
  (*kernel_fn)(*dev_ctx, *dense_out_grad, axis, dense_x_grad);

  return x_grad;
}

std::vector<Tensor> meshgrid_impl(const std::vector<Tensor>& inputs) {
  Backend kernel_backend = Backend::UNDEFINED;
  DataLayout kernel_layout = DataLayout::UNDEFINED;
  DataType kernel_data_type = DataType::UNDEFINED;

  if (kernel_backend == Backend::UNDEFINED ||
      kernel_layout == DataLayout::UNDEFINED ||
      kernel_data_type == DataType::UNDEFINED) {
    auto kernel_key_set = ParseKernelKeyByInputArgs(inputs);
    auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();
    if (kernel_backend == Backend::UNDEFINED) {
      kernel_backend = kernel_key.backend();
    }
    if (kernel_layout == DataLayout::UNDEFINED) {
      kernel_layout = kernel_key.layout();
    }
    if (kernel_data_type == DataType::UNDEFINED) {
      kernel_data_type = kernel_key.dtype();
    }
  }

  const auto& kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "meshgrid", {kernel_backend, kernel_layout, kernel_data_type});
  VLOG(6) << "meshgrid API kernel key: [" << kernel_backend << ", "
          << kernel_layout << ", " << kernel_data_type << "]";
  VLOG(6) << "meshgrid API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_backend);

  auto input_inputs_vec = PrepareData(inputs, kernel.InputAt(0), {});
  std::vector<const phi::DenseTensor*> input_inputs(input_inputs_vec->size());
  for (size_t i = 0; i < input_inputs.size(); ++i) {
    input_inputs[i] = &input_inputs_vec->at(i);
  }

  auto x_meta_vec = MakeMetaTensor(input_inputs);
  std::vector<phi::MetaTensor*> inputs_metas(x_meta_vec.size());
  for (size_t i = 0; i < x_meta_vec.size(); ++i) {
    inputs_metas[i] = &x_meta_vec[i];
  }

  // Calculate the number of out tensors
  size_t out_number = inputs.size();

  std::vector<Tensor> out;
  auto dense_outs = SetKernelOutput(out_number, kernel_backend, &out);

  std::vector<phi::MetaTensor> meta_outs;
  meta_outs.reserve(out_number);
  std::vector<phi::MetaTensor*> meta_out_ptrs;
  meta_out_ptrs.reserve(out_number);
  for (size_t i = 0; i < out_number; ++i) {
    meta_outs.push_back(dense_outs[i]);
    meta_out_ptrs.push_back(&meta_outs.back());
  }
  phi::MeshgridInferMeta(inputs_metas, meta_out_ptrs);

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const std::vector<const phi::DenseTensor*>&,
                                    std::vector<phi::DenseTensor*>&);
  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();
  (*kernel_fn)(*dev_ctx, input_inputs, dense_outs);

  return out;
}

std::vector<Tensor> meshgrid_grad_impl(
    const std::vector<Tensor>& inputs,
    const std::vector<Tensor>& outputs_grad) {
  Backend kernel_backend = Backend::UNDEFINED;
  DataLayout kernel_layout = DataLayout::UNDEFINED;
  DataType kernel_data_type = DataType::UNDEFINED;

  if (kernel_backend == Backend::UNDEFINED ||
      kernel_layout == DataLayout::UNDEFINED ||
      kernel_data_type == DataType::UNDEFINED) {
    auto kernel_key_set = ParseKernelKeyByInputArgs(inputs, outputs_grad);
    auto kernel_key = kernel_key_set.GetHighestPriorityKernelKey();
    if (kernel_backend == Backend::UNDEFINED) {
      kernel_backend = kernel_key.backend();
    }
    if (kernel_layout == DataLayout::UNDEFINED) {
      kernel_layout = kernel_key.layout();
    }
    if (kernel_data_type == DataType::UNDEFINED) {
      kernel_data_type = kernel_key.dtype();
    }
  }

  const auto& kernel = phi::KernelFactory::Instance().SelectKernelOrThrowError(
      "meshgrid_grad", {kernel_backend, kernel_layout, kernel_data_type});
  VLOG(6) << "meshgrid_grad API kernel key: [" << kernel_backend << ", "
          << kernel_layout << ", " << kernel_data_type << "]";
  VLOG(6) << "meshgrid_grad API kernel: " << kernel;

  auto* dev_ctx = GetDeviceContextByBackend(kernel_backend);

  auto input_inputs_vec = PrepareData(inputs, kernel.InputAt(0), {});
  std::vector<const phi::DenseTensor*> input_inputs(input_inputs_vec->size());
  for (size_t i = 0; i < input_inputs.size(); ++i) {
    input_inputs[i] = &input_inputs_vec->at(i);
  }
  auto input_outputs_grad_vec =
      PrepareData(outputs_grad, kernel.InputAt(1), {});
  std::vector<const phi::DenseTensor*> input_outputs_grad(
      input_outputs_grad_vec->size());
  for (size_t i = 0; i < input_outputs_grad.size(); ++i) {
    input_outputs_grad[i] = &input_outputs_grad_vec->at(i);
  }

  size_t out_number = inputs.size();
  std::vector<Tensor> api_output;
  auto kernel_out = SetKernelOutput(out_number, kernel_backend, &api_output);

  auto inputs_meta_vec = MakeMetaTensor(input_inputs);
  std::vector<phi::MetaTensor*> inputs_metas(inputs_meta_vec.size());
  for (size_t i = 0; i < inputs_meta_vec.size(); ++i) {
    inputs_metas[i] = &inputs_meta_vec[i];
  }

  auto outputs_grad_meta_vec = MakeMetaTensor(input_outputs_grad);
  std::vector<phi::MetaTensor*> outputs_grad_metas(
      outputs_grad_meta_vec.size());
  for (size_t i = 0; i < outputs_grad_meta_vec.size(); ++i) {
    outputs_grad_metas[i] = &outputs_grad_meta_vec[i];
  }

  std::vector<phi::MetaTensor> meta_outs;
  meta_outs.reserve(out_number);
  std::vector<phi::MetaTensor*> meta_out_ptrs;
  meta_out_ptrs.reserve(out_number);
  for (size_t i = 0; i < out_number; ++i) {
    meta_outs.push_back(kernel_out[i]);
    meta_out_ptrs.push_back(&meta_outs.back());
  }

  phi::MeshgridGradInferMeta(inputs_metas, outputs_grad_metas, meta_out_ptrs);

  using kernel_signature = void (*)(const platform::DeviceContext&,
                                    const std::vector<const phi::DenseTensor*>&,
                                    const std::vector<const phi::DenseTensor*>&,
                                    std::vector<phi::DenseTensor*>&);
  auto* kernel_fn = kernel.GetVariadicKernelFn<kernel_signature>();
  (*kernel_fn)(*dev_ctx, input_inputs, input_outputs_grad, kernel_out);

  return api_output;
}

}  // namespace experimental
}  // namespace paddle
