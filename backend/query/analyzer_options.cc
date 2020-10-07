//
// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "backend/query/analyzer_options.h"

#include "zetasql/public/analyzer.h"
#include "zetasql/public/options.pb.h"
#include "absl/time/time.h"
#include "common/constants.h"
#include "common/feature_flags.h"

namespace google {
namespace spanner {
namespace emulator {
namespace backend {

zetasql::AnalyzerOptions MakeGoogleSqlAnalyzerOptions() {
  zetasql::AnalyzerOptions options;
  absl::TimeZone time_zone;
  absl::LoadTimeZone(kDefaultTimeZone, &time_zone);
  options.set_default_time_zone(time_zone);
  options.set_error_message_mode(
      zetasql::AnalyzerOptions::ERROR_MESSAGE_MULTI_LINE_WITH_CARET);

  options.set_language(MakeGoogleSqlLanguageOptions());

  options.set_allow_undeclared_parameters(true);

  // Spanner does not support positional parameters, so tell ZetaSQL to always
  // use named parameter bindings.
  options.set_parameter_mode(zetasql::PARAMETER_NAMED);

  return options;
}

zetasql::LanguageOptions MakeGoogleSqlLanguageOptions() {
  zetasql::LanguageOptions options;

  options.set_name_resolution_mode(zetasql::NAME_RESOLUTION_DEFAULT);
  options.set_product_mode(zetasql::PRODUCT_EXTERNAL);
  options.SetEnabledLanguageFeatures({
      zetasql::FEATURE_TIMESTAMP_NANOS,
      // TODO: Reenable zetasql::FEATURE_TABLESAMPLE once AST
      // filtering lands.
      zetasql::FEATURE_V_1_1_HAVING_IN_AGGREGATE,
      zetasql::FEATURE_V_1_1_NULL_HANDLING_MODIFIER_IN_AGGREGATE,
      zetasql::FEATURE_V_1_2_SAFE_FUNCTION_CALL,
      zetasql::FEATURE_V_1_1_ORDER_BY_COLLATE,
  });
  options.SetSupportedStatementKinds({
      zetasql::RESOLVED_QUERY_STMT,
      zetasql::RESOLVED_INSERT_STMT,
      zetasql::RESOLVED_UPDATE_STMT,
      zetasql::RESOLVED_DELETE_STMT,
  });

  if (EmulatorFeatureFlags::instance().flags().enable_numeric_type) {
    options.EnableLanguageFeature(zetasql::FEATURE_NUMERIC_TYPE);
  }

  return options;
}

static void DisableOption(zetasql::LanguageFeature feature,
                          zetasql::LanguageOptions* options) {
  std::set<zetasql::LanguageFeature> features(
      options->GetEnabledLanguageFeatures());
  features.erase(feature);
  options->SetEnabledLanguageFeatures(features);
}

zetasql::LanguageOptions MakeGoogleSqlLanguageOptionsForCompliance() {
  auto options = MakeGoogleSqlLanguageOptions();
  DisableOption(zetasql::FEATURE_ANALYTIC_FUNCTIONS, &options);
  DisableOption(zetasql::FEATURE_TABLESAMPLE, &options);
  return options;
}

}  // namespace backend
}  // namespace emulator
}  // namespace spanner
}  // namespace google
