BEGIN {
  FS = ",";
}

{
  if(NF != 5){
    print $0;
  }
}
