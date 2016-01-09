program test(in, out, err);
var arr : array [1..9] of array [1..9] of integer;
begin
    arr[1][1] := 1234;
    arr[1][2] := 5678;
    arr[2][1] := 1324;
    arr[2][2] := 5768;

    printInt(arr[1][1]+arr[1][2]+arr[2][1]+arr[2][2])
end.

