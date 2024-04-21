constmatrix=document.querySelector(".matrix")


function makeChar(){
    let characters="ABCDFGHEJKLMNOPQRSTUVWXYZ开儿艾诶开伊艾艾西吉艾艾伊娜伊爪ㄚれ丹爪モ工れ匚卄工れモちモabcdefghijklmnopqrstuvwxyz"
    return characters.charAt(Math.floor(Math.random() * characters.length));
}

function makeDigit(){
    let characters="100"
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

const clamp = (val, min=1, max=10) => Math.min(Math.max(val, min), max)

function genDigit() {
    const w = 40;
    const h = 40;

    for(let i = 0; i < w; i++)
    {
        var rand = Math.random() * 10;
        const timeout = Math.floor(rand) * 1000;
        let delay = 0;
        const column = document.createElement("p");
        column.className = "matrix-digit";
        
        for (let j = 0; j < h; j++)
        {
            const letter = document.createElement("span");
            letter.innerHTML = makeDigit();
            column.appendChild(letter);

            delay += 100;
            column.children[j].style.animationDelay = delay + "ms";
            column.children[j].style.margin = clamp(rand * i / 1.2, 0, 15) + `px`;
            column.children[j].style.fontSize = rand * j / 2.2 + 15 + `px`;
        }
        
        setTimeout(()=>
        {
            try{
                matrix.appendChild(column)
            }catch{
                // do nothing. time out is just being spicy
            }
        }, timeout);
    }
}

if(window.location.href.toLowerCase().includes("index.html"))
{
    genText();
}
else
{
    genDigit();
}