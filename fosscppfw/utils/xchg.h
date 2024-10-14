#pragma once

template<typename T> inline void xchg(T &x1, T &x2) { T aux(x1); x1 = x2; x2 = aux; }
template<typename T> inline void xchg(T &&x1, T &&x2) { T aux(std::move(x1)); x1 = std::move(x2); x2 = std::move(aux); }
