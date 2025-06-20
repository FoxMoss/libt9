let get_word = (path, index) => {
  let ptr = _malloc(path.length + 1);
  stringToUTF8(path, ptr, path.length + 1)
  let res = UTF8ToString(_get_word(ptr, index))
  _free(ptr);
  return res;
}

let str = "";
let current_word = [];
let index = 0;

let text_box = document.getElementById("text");
let rerender = () => {
  text_box.value = str + get_word(current_word.join(""), index);
}

for (let key = 1; key <= 9; key++) {
  document.getElementById(key.toString()).addEventListener("click", () => {
    current_word.push(key.toString());
    rerender();
    index = 0;
  });
}
document.getElementById("*").addEventListener("click", () => {
  index++;
  rerender();
});

document.getElementById("0").addEventListener("click", () => {
  str += get_word(current_word.join(""), index) + " ";
  current_word = [];
  index = 0;
});

document.getElementById("#").addEventListener("click", () => {
  current_word.pop();
  rerender();
  index = 0;
});

