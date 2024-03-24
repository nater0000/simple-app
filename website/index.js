constmatrix=document.querySelector(".matrix")


function makeChar(){
    let characters="ABCDFGHEJKLMNOPQRSTUVWXYZ开儿艾诶开伊艾艾西吉艾艾伊娜伊爪ㄚれ丹爪モ工れ匚卄工れモちモabcdefghijklmnopqrstuvwxyz"
    return characters.charAt(Math.floor(Math.random() * characters.length));
}

function genText() {
    const w = 45;
    const h = 50;

    for(let i = 0; i < w; i++)
    {
        const timeout = Math.floor(Math.random() * 10) * 1000;
        let delay = 0;
        const column = document.createElement("p");
        column.className = "matrix-text";
        for (let j = 0; j < h; j++)
        {
            const letter = document.createElement("span");
            letter.innerHTML = makeChar();
            column.appendChild(letter);

            delay += 100;
            column.children[j].style.animationDelay = delay + "ms";
        }

        setTimeout(()=>
        {
            matrix.appendChild(column)
        }, timeout);
    }
}

genText();