class BackButton extends HTMLElement {
    connectedCallback() {
        this.innerHTML = `<a href="javascript:history.back()"><button class="table-btn">Back</button></a>`
    }
}

customElements.define('back-btn', BackButton)