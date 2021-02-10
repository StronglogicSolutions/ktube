#!/usr/bin/env bash

third_party/gn/out/gn gen out
ninja -C out

patchelf --replace-needed libcpr.so.1 "$PWD"/third_party/cpr/lib/libcpr.so out/ktube
patchelf --replace-needed libcpr.so.1 "$PWD"/third_party/cpr/lib/libcpr.so out/ut_ktube
