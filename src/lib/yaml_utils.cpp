/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "yaml_utils.h"

const unsigned int MAX_BUF = 10;
namespace libpagemaker
{
  char *getOutputValue(int value, int *printed)
  {
    std::fprintf(stderr, "Allocating.\n");
    char *buf = new char[MAX_BUF];
    int theoreticalPrinted = std::snprintf(buf, MAX_BUF, "%d", value);
    *printed = (theoreticalPrinted > (int)MAX_BUF) ? MAX_BUF : theoreticalPrinted;
    return buf;
  }

  /* This silly function is necessaryf because libyaml isn't const-correct. */
  char *getOutputValue(const char *value, int *printed)
  {
    int len = strlen(value) + 1;
    std::fprintf(stderr, "Allocating.\n");
    char *valOut = new char[len];
    if (valOut == NULL)
    {
      throw YamlException();
    }
    *printed = len - 1;
    strncpy(valOut, value, len);
    return valOut;
  }

  void yamlTryEmit(yaml_emitter_t *emitter,
    yaml_event_t *event)
  {
    if (!yaml_emitter_emit(emitter, event))
    {
      throw YamlException();
    }
  }

  void yamlBeginMap(yaml_emitter_t *emitter)
  {
    yaml_event_t event;
    if (!yaml_mapping_start_event_initialize(
      &event, NULL, NULL, 1, YAML_ANY_MAPPING_STYLE))
    {
      throw YamlException();
    }
    yamlTryEmit(emitter, &event);
    yaml_event_delete(&event);
  }

  void yamlEndMap(yaml_emitter_t *emitter)
  {
    yaml_event_t event;
    if (!yaml_mapping_end_event_initialize(&event))
    {
      throw YamlException();
    }
    yamlTryEmit(emitter, &event);
  }
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
