/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cstdio>
#include <yaml.h>

namespace libpagemaker
{
class YamlException
{
};

char *getOutputValue(int value, int *printed);
char *getOutputValue(const char *value, int *printed);
char *getOutputValue(bool value, int *printed);
void yamlTryEmit(yaml_emitter_t *emitter,
                 yaml_event_t *event);
void yamlBeginMap(yaml_emitter_t *emitter);

template <typename VALUE> void yamlTryPutScalar(
  yaml_emitter_t *emitter, VALUE value)
{
  int printed;
  char *output = getOutputValue(value, &printed);
  if (printed < 0 || !output)
  {
    throw YamlException();
  }
  yaml_event_t event;
  if (!yaml_scalar_event_initialize(&event,
                                    NULL, NULL, (unsigned char *)(output), printed, 1, 0,
                                    YAML_ANY_SCALAR_STYLE))
  {
    throw YamlException();
  }
  yamlTryEmit(emitter, &event);
  delete[] output;
}

template <typename VALUE> void yamlMapEntry(
  yaml_emitter_t *emitter, const char *key, VALUE value)
{
  yamlTryPutScalar(emitter, key);
  yamlTryPutScalar(emitter, value);
}

template <typename OBJECT> void yamlMapObject(
  yaml_emitter_t *emitter, const char *key, const OBJECT &object)
{
  yamlTryPutScalar(emitter, key);
  object.emitYaml(emitter);
}

void yamlEndMap(yaml_emitter_t *emitter);

template <typename SEQUENCE> void yamlForeach(
  yaml_emitter_t *emitter, const char *key, const SEQUENCE &value)
{
  yamlTryPutScalar(emitter, key);

  yaml_event_t event;
  if (!yaml_sequence_start_event_initialize(
        &event, NULL, NULL, 1, YAML_ANY_SEQUENCE_STYLE))
  {
    throw YamlException();
  }
  yamlTryEmit(emitter, &event);
  for (unsigned i = 0; i < value.size(); ++i)
  {
    value[i].emitYaml(emitter);
  }
  if (!yaml_sequence_end_event_initialize(&event))
  {
    throw YamlException();
  }
  yamlTryEmit(emitter, &event);
}

template <typename SEQUENCE> void yamlIndirectForeach(
  yaml_emitter_t *emitter, const char *key, const SEQUENCE &value)
{
  yamlTryPutScalar(emitter, key);

  yaml_event_t event;
  if (!yaml_sequence_start_event_initialize(
        &event, NULL, NULL, 1, YAML_ANY_SEQUENCE_STYLE))
  {
    throw YamlException();
  }
  yamlTryEmit(emitter, &event);
  for (unsigned i = 0; i < value.size(); ++i)
  {
    value[i]->emitYaml(emitter);
  }
  if (!yaml_sequence_end_event_initialize(&event))
  {
    throw YamlException();
  }
  yamlTryEmit(emitter, &event);
}

template <typename DOCUMENT> void dumpAsYaml(FILE *file, const DOCUMENT &document)
{
  yaml_emitter_t emitter;
  yaml_emitter_initialize(&emitter);
  yaml_emitter_set_output_file(&emitter, file);
  yaml_event_t event;
  if (!yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING))
  {
    throw YamlException();
  }
  yamlTryEmit(&emitter, &event);
  if (!yaml_document_start_event_initialize(&event,
      NULL, NULL, NULL, 1))
  {
    throw YamlException();
  }
  yamlTryEmit(&emitter, &event);
  document.emitYaml(&emitter);
  if (!yaml_document_end_event_initialize(&event, 1))
  {
    throw YamlException();
  }
  yamlTryEmit(&emitter, &event);
  if (!yaml_stream_end_event_initialize(&event))
  {
    throw YamlException();
  }
  yamlTryEmit(&emitter, &event);
  yaml_emitter_delete(&emitter);
}
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
