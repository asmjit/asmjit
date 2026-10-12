// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/build_export_p.h>

#include <asmjit/axl/commons.h>
#include <asmjit/core/os_utils_p.h>

#if !defined(_WIN32)
  #include <fcntl.h>
  #include <unistd.h>
#endif

#if defined(__linux__)
  #include <link.h>
#endif

ASMJIT_BEGIN_NAMESPACE

#if !defined(_WIN32)
Error OSUtils::read_file(const char* name, String& dst, size_t max_size) noexcept {
  char* buffer = dst.prepare(String::ModifyOp::kAssign, max_size);
  if (ASMJIT_UNLIKELY(!buffer)) {
    return make_error(Error::kOutOfMemory);
  }

  int fd = ::open(name, O_RDONLY | O_CLOEXEC);
  if (fd < 0) {
    dst.clear();
    return make_error(Error::kFailedToOpenFile);
  }

  intptr_t len = ::read(fd, buffer, max_size);
  if (len >= 0) {
    buffer[len] = '\0';
    dst._set_size(size_t(len));
  }

  ::close(fd);
  return Error::kOk;
}
#endif

#if defined(__linux__)
struct ModuleRangeQuery {
  uintptr_t addr;
  uintptr_t start;
  uintptr_t end;
};

static int module_address_range_callback(struct dl_phdr_info* info, size_t, void* data) noexcept {
  ModuleRangeQuery* query = static_cast<ModuleRangeQuery*>(data);

  uintptr_t start = UINTPTR_MAX;
  uintptr_t end = 0;
  bool found = false;

  for (size_t i = 0; i < info->dlpi_phnum; i++) {
    const auto& phdr = info->dlpi_phdr[i];
    if (phdr.p_type != PT_LOAD) {
      continue;
    }

    uintptr_t segment_start = uintptr_t(info->dlpi_addr + phdr.p_vaddr);
    uintptr_t segment_end = segment_start + uintptr_t(phdr.p_memsz);

    start = axl::min(start, segment_start);
    end = axl::max(end, segment_end);
    found |= query->addr >= segment_start && query->addr < segment_end;
  }

  if (!found) {
    return 0;
  }

  query->start = start;
  query->end = end;
  return 1;
}

Error OSUtils::module_address_range(const void* addr, uintptr_t* start, uintptr_t* end) noexcept {
  ModuleRangeQuery query { uintptr_t(addr), 0u, 0u };
  if (!dl_iterate_phdr(module_address_range_callback, &query)) {
    return make_error(Error::kInvalidArgument);
  }

  *start = query.start;
  *end = query.end;
  return Error::kOk;
}
#else
Error OSUtils::module_address_range(const void* addr, uintptr_t* start, uintptr_t* end) noexcept {
  axl::maybe_unused(addr, start, end);
  return make_error(Error::kFeatureNotEnabled);
}
#endif

ASMJIT_END_NAMESPACE
