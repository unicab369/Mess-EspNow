class SharedHeader extends HTMLElement {
    connectedCallback() {
        this.innerHTML = `
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="icon" href="data:,">
        <link rel="stylesheet" type="text/css" href="../../style/sharedStyle.css">
        <link rel="stylesheet" type="text/css" href="../../style/picnic.min.css">
        `
    }
}

customElements.define('shared-header', SharedHeader)