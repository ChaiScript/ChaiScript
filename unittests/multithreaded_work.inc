def do_chai_work(num_iters, id)
{
  var i = 0;
  for (var k = 0; k<num_iters * 10; ++k)
  {
    if (k == 1000)
    {
      eval("def getid(id) : id == " + to_string(id) + " { return " + to_string(id) + "}");
    }
    i += k;
  }

  eval("def getvalue(id) : id == " + to_string(id) + " { return " + to_string(i) + "}");

}
