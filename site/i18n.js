const SUPPORTED_LANGS = ["en", "zh"];
window.i18nData = {};
let currentLang = "en";

function translatePage() {
  document.querySelectorAll("[data-i18n]").forEach((el) => {
    const key = el.getAttribute("data-i18n");
    if (window.i18nData[key]) {
      el.innerHTML = window.i18nData[key];
    }
  });
  document.getElementById("pay-info").innerText = "";
}

async function loadLocale(lang) {
  if (!SUPPORTED_LANGS.includes(lang)) lang = "en";
  currentLang = lang;
  const response = await fetch(`locales/${lang}.json`);
  window.i18nData = await response.json();
  translatePage();
}

window.initI18n = function () {
  const sel = document.getElementById("lang-selector");
  sel.value = currentLang;
  sel.onchange = () => loadLocale(sel.value);
  loadLocale(currentLang);
};
