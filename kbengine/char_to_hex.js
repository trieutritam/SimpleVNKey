var data = [
  ['A','a','¸','¸','µ','µ','¶','¶','·','·','¹','¹',],       // 0: a
  ['¢','©','Ê','Ê','Ç','Ç','È','È','É','É','Ë','Ë',], 		// 1: a^
  ['¡','¨','¾','¾','»','»','¼','¼','½','½','Æ','Æ',], 		// 2: a(
  ['B','b','C','c','D','d',], 
  ['§','®',], 
  ['E','e','Ð','Ð','Ì','Ì','Î','Î','Ï','Ï','Ñ','Ñ',], 		// 3: e
  ['£','ª','Õ','Õ','Ò','Ò','Ó','Ó','Ô','Ô','Ö','Ö',],       // 4: e^
  ['F','f','G','g','H','h',], 
  ['I','i','Ý','Ý','×','×','Ø','Ø','Ü','Ü','Þ','Þ',], 		// 5: i
  ['J','j','K','k','L','l','M','m','N','n',], 
  ['O','o','ã','ã','ß','ß','á','á','â','â','ä','ä',], 		// 6: o
  ['¤','«','è','è','å','å','æ','æ','ç','ç','é','é',], 		// 7: o^
  ['¥','¬','í','í','ê','ê','ë','ë','ì','ì','î','î',], 		// 8: o+
  ['P','p','Q','q','R','r','S','s','T','t',], 
  ['U','u','ó','ó','ï','ï','ñ','ñ','ò','ò','ô','ô',], 		// 9: u
  ['¦','­','ø','ø','õ','õ','ö','ö','÷','÷','ù','ù',], 		//10: u+ 
  ['V','v','W','w','X','x',], 
  ['Y','y','ý','ý','ú','ú','û','û','ü','ü','þ','þ',], 		//11: y
  ['Z','z',], 
];

data.forEach(arr => {
  var result = arr.map( c => {
    let val = Number(c.charCodeAt(0)).toString(16);
    if (val.length > 2)
      return '0x' + val;
    else
      return '0x00' + val;
  }).join(", ")
  
  console.log(result);
});
