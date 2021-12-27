{
  val s = "abc"
  var n = 0
  ;; Sum the values of the bytes in s, except 'b'.
  for c in s {
    if c == 'b' then continue
    n = n + c
  }

  ;; Result.
  n
}
