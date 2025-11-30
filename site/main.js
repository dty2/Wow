window.addEventListener("DOMContentLoaded", () => {
  if (window.initI18n) window.initI18n();

  // quick start
  document.getElementById("quick-start-btn").onclick = () => {
    document.getElementById("quick-start-modal").style.display = "flex";
  };
  document.getElementById("quick-start-close").onclick = () => {
    document.getElementById("quick-start-modal").style.display = "none";
  };

  // whipping / milk tea
  document.getElementById("support-btn").onclick = () => {
    document.getElementById("support-modal").style.display = "flex";
    document.getElementById("pay-info").innerText = "";
  };
  document.getElementById("support-close").onclick = () => {
    document.getElementById("support-modal").style.display = "none";
    document.getElementById("pay-info").innerText = "";
  };

  document.querySelectorAll(".support-pay-btn").forEach((btn) => {
    btn.onclick = function () {
      const action = this.dataset.action;
      let msg = "";
      if (action === "whip") {
        msg =
          window.i18nData?.support_pay_whip || "请扫码或支付1元催更（仅演示）";
      } else if (action === "coffee") {
        msg =
          window.i18nData?.support_pay_coffee ||
          "请扫码支付12元买奶茶（仅演示）";
      }
      document.getElementById("pay-info").innerText = msg;
    };
  });

  document.querySelectorAll(".modal").forEach((modal) => {
    modal.addEventListener("click", function (e) {
      if (e.target === this) this.style.display = "none";
    });
  });
});
