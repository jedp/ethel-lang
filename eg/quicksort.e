; Test quicksort.

; Swap i and j in bytearray a.
val swap = fn(a, i, j) {
  val temp = a[i]
  a[i] = a[j]
  a[j] = temp
}

; Partition bytearray a between hi and lo.
val partition = fn(a, lo, hi) {
  val pivot = a[hi]
  var i = lo
  for j in lo..hi {
    if s[j] < pivot then {
      swap(a, i, j)
      i = i + 1
    }
  }
  swap(a, i, hi)
  i
}

; Sort bytearray a between indices lo and hi inclusive.
val quicksort = fn(a, lo, hi) {
  if (lo < hi) then {
    var p = partition(a, lo, hi)
    quicksort(a, lo, p - 1)
    quicksort(a, p + 1, hi)
  }
}

val s = input("A string, please:")
quicksort(s, 0, s.length() - 1)
print("Sorted:", s)

