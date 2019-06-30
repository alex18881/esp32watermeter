var app = new Vue({
	el: '#app',
	data: function () {
		return {
			currentWifi: null,
			enterPasswordOpen: false,
			password: "",
			current: null,
			wifis: []
		};
	},
	methods: {
		openPasskeyForm: function (wifi) {
			this.currentWifi = wifi;
			this.enterPasswordOpen = true;
		},
		closePasskeyForm: function () {
			this.enterPasswordOpen = false;
		},

		connect: function () {
			axios.post(
				"/api/wifi-connect",
				"ssid=" + encodeURIComponent(this.currentWifi.ssid) + "&passkey=" + encodeURIComponent(this.password),
				{ headers: { 'content-type': 'application/x-www-form-urlencoded' } }
			)
				.then(function () {
					this.password = "";
					alert("Сохранено. Перезагрузка");
					window.setTimeout(this.loadWifis.bind(this), 10000);
				}.bind(this))
				.catch(function (err) {
					alert(err);
				}.bind(this))
		},

		loadWifis: function () {
			axios.get("/api/wifi-list")
				.then(function (data) {
					this.current = data.data.connected;
					this.wifis.push.apply(this.wifis, data.data.listAvailable);
					//alert(data);
				}.bind(this))
				.catch(function (err) {
					alert(err);
				}.bind(this));
		}
	},
	mounted: function() {
		this.loadWifis();
	}
});