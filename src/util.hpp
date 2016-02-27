#pragma once

namespace rev {

  inline std::string replace(const std::string& s, char a, char b) {

    std::string out(s);

    int pos = 0;
    while ((pos = out.find(a)) != std::string::npos) {
      out[pos] = b;
    }

    return out;
  }
}
