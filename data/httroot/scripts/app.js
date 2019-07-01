var app = new Vue({
	el: '#app',
	data: function () {
		return {
			settings: {
				activeTab: 0,
				currentWifi: null,
				enterPasswordOpen: false,
				password: "",
				current: null,
				wifis: [],
				counters: {
					value1: 0,
					value2: 0
				}
			}
		};
	},
	methods: {
		selectSettinsTab: function (val) {
			this.settings.activeTab = val;
			if (val === 0) {
				this.loadWifis();
			}

		},
		openPasskeyForm: function (wifi) {
			this.settings.currentWifi = wifi;
			this.settings.enterPasswordOpen = true;
		},
		closePasskeyForm: function () {
			this.settings.enterPasswordOpen = false;
		},

		connect: function () {
			axios.post(
				"/api/wifi-connect",
				"ssid=" + encodeURIComponent(this.settings.currentWifi.ssid) + "&passkey=" + encodeURIComponent(this.settings.password),
				{ headers: { 'content-type': 'application/x-www-form-urlencoded' } }
			)
				.then(function () {
					this.settings.password = "";
					alert("Сохранено. Перезагрузка");
					window.setTimeout(this.selectSettinsTab.bind(this, this.settings.activeTab), 10000);
				}.bind(this))
				.catch(function (err) {
					alert(err);
				}.bind(this))
		},

		saveValues: function () {
			var val1 = String(this.settings.value1).split("."),
				val2 = String(this.settings.value2).split(".");

			axios.post(
				"/api/values-update",
				"val1=" + (val1[0] || 0) + "&dec1=" + (val1[1] || 0) + "&val2=" + (val2[0] || 0) + "&dec2=" + (val2[1] || 0),
				{ headers: { 'content-type': 'application/x-www-form-urlencoded' } }
			)
				.then(function () {
					alert("Сохранено. Перезагрузка");
					window.setTimeout(this.selectSettinsTab.bind(this, this.settings.activeTab), 10000);
				}.bind(this))
				.catch(function (err) {
					alert(err);
				}.bind(this))
		},

		loadWifis: function () {
			axios.get("/api/wifi-list")
				.then(function (data) {
					this.settings.current = data.data.connected;
					this.settings.wifis.push.apply(this.settings.wifis, data.data.listAvailable);
					//alert(data);
				}.bind(this))
				.catch(function (err) {
					alert(err);
				}.bind(this));
		}
	},
	mounted: function() {
		this.selectSettinsTab(0);
	}
});