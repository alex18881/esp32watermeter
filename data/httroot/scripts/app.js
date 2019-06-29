var app = new Vue({
	el: '#app',
	data: function () {
		return {
			wifis: []
		};
	},
	methods: {
		loadWifis: function () {
			axios.get("/api/wifi-list")
				.then(function (data) {
					this.wifis.push.apply(this.wifis, data.data);
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