void screenUpdate() {
  display.clear(); 
  display.drawStringMaxWidth(0,0,122, metaDataTxt); 
  display.display();
}


int usedBuffer() {
  return(( DATA_BUFFER_SIZE + writePointer - readPointer ) % DATA_BUFFER_SIZE);
}

