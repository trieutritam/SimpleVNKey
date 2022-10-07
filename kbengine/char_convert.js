const text = `
u
uc
ui
um
un
ung
up
ut
uu
`;


const list = text.split("\n").sort();
const filter = "KEY_Y"

for(let item of list) {
  if (item === "") continue;
  const arr = Array.from(item);
  const combine = arr.map((c, i) => {
    return "KEY_" + c.toUpperCase();
  });
  
  // if (combine[0] !== filter) continue;

  console.log("{ " + combine.join(", ") + " },");
}

