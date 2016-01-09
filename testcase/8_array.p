program test(in, out, err);
var arr : array [1..9] of integer;
begin
    arr[1] := 1234;
    arr[2] := 5678;

    printInt(arr[1]+arr[2])
end.

