const ws = new WebSocket(`ws://${location.hostname}/ws`);

ws.onmessage = (event) => {
    const events = JSON.parse(event.data);
    const list = document.getElementById('events');
    list.innerHTML = '';
    events.forEach(e => {
        const li = document.createElement('li');
        li.textContent = `${e.time} - ${e.name}`;
        list.appendChild(li);
    });
};

// Formulaire d'ajout
document.getElementById('eventForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    const formData = new FormData(e.target);
    const data = {
        name: formData.get('name'),
        time: formData.get('time')
    };

    await fetch('/add', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(data)
    });

    e.target.reset();
});
